using SnoreSaber.Core;
using SnoreSaber.Core.Gameplay;
using SnoreSaber.Features.Leaderboards.Domain;
using SnoreSaber.Features.Replays.Format;
using SnoreSaber.Live.V1;
using System;
using System.Collections.Generic;
using ReplayNoteEventSource = SnoreSaber.Features.Replays.Format.NoteEvent;
using ReplayNoteEventTypeSource = SnoreSaber.Features.Replays.Format.NoteEventType;
using ReplayPoseGroupSource = SnoreSaber.Features.Replays.Format.VRPoseGroup;
using ReplayPoseSource = SnoreSaber.Features.Replays.Format.VRPose;
using ReplayPositionSource = SnoreSaber.Features.Replays.Format.VRPosition;
using ReplayQuaternionSource = SnoreSaber.Features.Replays.Format.VRRotation;

namespace SnoreSaber.Features.Live.Replay {
    internal partial class LiveReplayStreamingService {
        private ReplayCursor Cursor(ulong sequence, float songTime) {
            return new ReplayCursor {
                Sequence = sequence,
                SongTimeMs = (long)Math.Round(songTime * 1000f),
                ClientTimeUnixMs = UnixNowMs()
            };
        }

        private PlayerIdentity PlayerIdentity() {
            return new PlayerIdentity {
                PlayerId = _ludus.SnoreSaberPlayerId,
                Platform = PlatformFromAuthType(_ludus.LocalAuthType),
                GameVersion = _runtimeInfo.GameVersion.ToString(),
                ClientVersion = _runtimeInfo.PluginVersion.ToString(),
                ReplayModVersion = _runtimeInfo.PluginVersion.ToString()
            };
        }

        private BeatmapIdentity BeatmapIdentity() {
            string levelId = _metadata.LevelID ?? string.Empty;
            string mapHash = ExtractMapHash(levelId);

            return new BeatmapIdentity {
                MapHash = mapHash,
                LevelId = levelId,
                Difficulty = _metadata.Difficulty,
                DifficultyName = DifficultyName(_metadata.Difficulty),
                Characteristic = _metadata.Characteristic ?? string.Empty,
                LeaderboardId = ExtractLeaderboardId(levelId),
                Modifiers = new List<string>(_metadata.Modifiers ?? new string[0]),
                MaxScore = _lastMaxScore
            };
        }

        private StreamReplayMetadata StreamMetadata() {
            return new StreamReplayMetadata {
                ReplayVersion = _metadata.Version?.ToString() ?? string.Empty,
                LevelId = _metadata.LevelID ?? string.Empty,
                Difficulty = _metadata.Difficulty,
                Characteristic = _metadata.Characteristic ?? string.Empty,
                Environment = _metadata.Environment ?? string.Empty,
                Modifiers = new List<string>(_metadata.Modifiers ?? new string[0]),
                NoteSpawnOffset = _metadata.NoteSpawnOffset,
                LeftHanded = _metadata.LeftHanded,
                InitialHeight = _metadata.InitialHeight,
                RoomRotation = _metadata.RoomRotation,
                RoomCenter = ToReplayVector(_metadata.RoomCenter),
                FailTimeSeconds = _metadata.FailTime,
                GameVersion = _metadata.GameVersion?.ToString() ?? string.Empty,
                PluginVersion = _metadata.PluginVersion?.ToString() ?? string.Empty,
                Platform = _metadata.Platform ?? string.Empty,
                SongSpeed = _metadata.SongSpeed > 0f ? _metadata.SongSpeed : 1f,
                JumpDistance = _metadata.JumpDistance,
                LeftSaberColor = ToReplayColor(_metadata.LeftSaberColor),
                RightSaberColor = ToReplayColor(_metadata.RightSaberColor)
            };
        }

        private static List<ReplayExtension> ToReplayExtensions(List<ReplayExtensionEntry> entries) {
            var extensions = new List<ReplayExtension>(entries.Count);
            foreach (ReplayExtensionEntry entry in entries) {
                extensions.Add(ToReplayExtension(entry));
            }
            return extensions;
        }

        private static ReplayExtension ToReplayExtension(ReplayExtensionEntry entry) {
            return new ReplayExtension {
                Id = entry.Id,
                Version = (uint)entry.Version,
                Payload = entry.Payload
            };
        }

        private ReplayScoreSummary ScoreSummary(LevelCompletionResults results) {
            uint maxScore = _lastMaxScore;
            double accuracy = maxScore > 0 ? Math.Min(1d, Math.Max(0d, (double)results.modifiedScore / maxScore)) : 0d;

            return new ReplayScoreSummary {
                Score = ToUint(results.multipliedScore),
                ModifiedScore = ToUint(results.modifiedScore),
                MaxScore = maxScore,
                Accuracy = accuracy,
                Combo = ToUint(results.maxCombo),
                MaxCombo = ToUint(results.maxCombo),
                FullCombo = results.fullCombo,
                GoodCuts = ToUint(results.goodCutsCount),
                BadCuts = ToUint(results.badCutsCount),
                MissedNotes = ToUint(results.missedCount)
            };
        }

        private static ReplayPoseFrame ToReplayPoseFrame(ReplayPoseGroupSource frame) {
            return new ReplayPoseFrame {
                Head = ToReplayPose(frame.Head),
                Left = ToReplayPose(frame.Left),
                Right = ToReplayPose(frame.Right),
                Fps = frame.FPS,
                TimeSeconds = frame.Time
            };
        }

        private static ReplayPose ToReplayPose(ReplayPoseSource pose) {
            return new ReplayPose {
                Position = ToReplayVector(pose.Position),
                Rotation = ToReplayQuaternion(pose.Rotation)
            };
        }

        private static ReplayNoteEvent ToReplayNoteEvent(ReplayNoteEventSource note) {
            var noteId = new ReplayNoteId {
                TimeSeconds = note.NoteID.Time,
                LineLayer = note.NoteID.LineLayer,
                LineIndex = note.NoteID.LineIndex,
                ColorType = note.NoteID.ColorType,
                CutDirection = note.NoteID.CutDirection
            };
            if (note.NoteID.GameplayType.HasValue) {
                noteId.GameplayType = note.NoteID.GameplayType.Value;
            }
            if (note.NoteID.ScoringType.HasValue) {
                noteId.ScoringType = note.NoteID.ScoringType.Value;
            }
            if (note.NoteID.CutDirectionAngleOffset.HasValue) {
                noteId.CutDirectionAngleOffset = note.NoteID.CutDirectionAngleOffset.Value;
            }

            var replayNote = new ReplayNoteEvent {
                NoteId = noteId,
                EventType = ToReplayNoteEventType(note.EventType),
                CutPoint = ToReplayVector(note.CutPoint),
                CutNormal = ToReplayVector(note.CutNormal),
                SaberDirection = ToReplayVector(note.SaberDirection),
                SaberType = note.SaberType,
                DirectionOk = note.DirectionOK,
                SaberSpeed = note.SaberSpeed,
                CutAngle = note.CutAngle,
                CutDistanceToCenter = note.CutDistanceToCenter,
                CutDirectionDeviation = note.CutDirectionDeviation,
                BeforeCutRating = note.BeforeCutRating,
                AfterCutRating = note.AfterCutRating,
                TimeSeconds = note.Time,
                UnityTimescale = note.UnityTimescale,
                TimeSyncTimescale = note.TimeSyncTimescale,
                WorldRotation = ToReplayQuaternion(note.WorldRotation),
                InverseWorldRotation = ToReplayQuaternion(note.InverseWorldRotation),
                NoteRotation = ToReplayQuaternion(note.NoteRotation),
                NotePosition = ToReplayVector(note.NotePosition)
            };
            if (note.TimeDeviation.HasValue) {
                replayNote.TimeDeviation = note.TimeDeviation.Value;
            }

            return replayNote;
        }

        private static ReplayVector3 ToReplayVector(ReplayPositionSource position) {
            return new ReplayVector3 {
                X = position.X,
                Y = position.Y,
                Z = position.Z
            };
        }

        private static ReplayVector3 ToReplayVector(ReplayPositionSource? position) {
            return position.HasValue ? ToReplayVector(position.Value) : new ReplayVector3();
        }

        private static ReplayQuaternion ToReplayQuaternion(ReplayQuaternionSource rotation) {
            return new ReplayQuaternion {
                X = rotation.X,
                Y = rotation.Y,
                Z = rotation.Z,
                W = rotation.W
            };
        }

        private static ReplayQuaternion ToReplayQuaternion(ReplayQuaternionSource? rotation) {
            return rotation.HasValue ? ToReplayQuaternion(rotation.Value) : new ReplayQuaternion();
        }

        private static ReplayColor ToReplayColor(UnityEngine.Color? color) {
            if (!color.HasValue) {
                return null;
            }

            var value = color.Value;
            return new ReplayColor {
                R = value.r,
                G = value.g,
                B = value.b,
                A = value.a
            };
        }

        private static ReplayNoteEventType ToReplayNoteEventType(ReplayNoteEventTypeSource type) {
            switch (type) {
                case ReplayNoteEventTypeSource.GoodCut:
                    return ReplayNoteEventType.ReplayNoteEventTypeGoodCut;
                case ReplayNoteEventTypeSource.BadCut:
                    return ReplayNoteEventType.ReplayNoteEventTypeBadCut;
                case ReplayNoteEventTypeSource.Miss:
                    return ReplayNoteEventType.ReplayNoteEventTypeMiss;
                case ReplayNoteEventTypeSource.Bomb:
                    return ReplayNoteEventType.ReplayNoteEventTypeBomb;
                default:
                    return ReplayNoteEventType.ReplayNoteEventTypeUnspecified;
            }
        }

        private static ReplayCompletion CompletionFromResults(LevelCompletionResults results, SnoreSaberPlayOutcome? playOutcomeOverride) {
            if (playOutcomeOverride.HasValue) {
                return CompletionFromPlayOutcome(playOutcomeOverride.Value);
            }

            if (results.levelEndAction == LevelCompletionResults.LevelEndAction.Quit) {
                return ReplayCompletion.ReplayCompletionQuit;
            }

            if (results.levelEndAction == LevelCompletionResults.LevelEndAction.Restart) {
                return ReplayCompletion.ReplayCompletionAborted;
            }

            if (results.levelEndStateType == LevelCompletionResults.LevelEndStateType.Failed) {
                return ReplayCompletion.ReplayCompletionFailed;
            }

            if (results.levelEndStateType == LevelCompletionResults.LevelEndStateType.Cleared) {
                return ReplayCompletion.ReplayCompletionPassed;
            }

            return ReplayCompletion.ReplayCompletionAborted;
        }

        private static ReplayCompletion CompletionFromPlayOutcome(SnoreSaberPlayOutcome outcome) {
            switch (outcome) {
                case SnoreSaberPlayOutcome.Clear:
                    return ReplayCompletion.ReplayCompletionPassed;
                case SnoreSaberPlayOutcome.Fail:
                    return ReplayCompletion.ReplayCompletionFailed;
                case SnoreSaberPlayOutcome.Quit:
                    return ReplayCompletion.ReplayCompletionQuit;
                case SnoreSaberPlayOutcome.Restart:
                    return ReplayCompletion.ReplayCompletionAborted;
                default:
                    return ReplayCompletion.ReplayCompletionUnspecified;
            }
        }

        private static ReplayPlatform PlatformFromAuthType(string authType) {
            switch (authType) {
                case "0":
                    return ReplayPlatform.ReplayPlatformSteam;
                case "1":
                    return ReplayPlatform.ReplayPlatformOculusPc;
                case "3":
                    return ReplayPlatform.ReplayPlatformDev;
                default:
                    return ReplayPlatform.ReplayPlatformUnspecified;
            }
        }

        private static string ExtractMapHash(string levelId) {
            string songHash;
            return SnoreSaberBeatmapKey.TryGetSongHash(levelId, out songHash) ? songHash : string.Empty;
        }

        private static string ExtractLeaderboardId(string levelId) {
            return ExtractMapHash(levelId);
        }

        private bool CanUsePublicPresenceForCurrentLevel() {
            string levelId = _metadata.LevelID;
            return SnoreSaberBeatmapKey.IsCustomLevelId(levelId)
                && !SnoreSaberBeatmapKey.IsWipLevelId(levelId)
                && SnoreSaberBeatmapKey.TryGetSongHash(levelId, out _);
        }

        private static string DifficultyName(int difficulty) {
            switch (difficulty) {
                case 1:
                    return "Easy";
                case 3:
                    return "Normal";
                case 5:
                    return "Hard";
                case 7:
                    return "Expert";
                case 9:
                    return "ExpertPlus";
                default:
                    return difficulty.ToString();
            }
        }

        private static ReplayEventCounts CloneCounts(ReplayEventCounts counts) {
            return new ReplayEventCounts {
                PoseFrames = counts.PoseFrames,
                HeightEvents = counts.HeightEvents,
                NoteEvents = counts.NoteEvents,
                ScoreEvents = counts.ScoreEvents,
                ComboEvents = counts.ComboEvents,
                MultiplierEvents = counts.MultiplierEvents,
                EnergyEvents = counts.EnergyEvents,
                PauseEvents = counts.PauseEvents
            };
        }

        private static uint ToUint(int value) {
            return value > 0 ? (uint)value : 0;
        }

        private long UnixNowMs() {
            return _clock.UnixTimeMilliseconds();
        }
    }
}
