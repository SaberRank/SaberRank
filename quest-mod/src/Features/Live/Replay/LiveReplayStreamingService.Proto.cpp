#include "Features/Live/Replay/LiveReplayStreamingService.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Features/Live/Ludus/Services/LudusSessionService.hpp"

#include <algorithm>
#include <cmath>

namespace SnoreSaber::Features::Live::Replay
{
    V1::ReplayCursor LiveReplayStreamingService::Cursor(uint64_t sequence, float songTime)
    {
        V1::ReplayCursor cursor{};
        cursor.Sequence = sequence;
        cursor.SongTimeMs = static_cast<int64_t>(std::llround(songTime * 1000.0f));
        cursor.ClientTimeUnixMs = UnixNowMs();
        return cursor;
    }

    V1::PlayerIdentity LiveReplayStreamingService::PlayerIdentity()
    {
        V1::PlayerIdentity player{};
        player.PlayerId = _ludus->SnoreSaberPlayerId();
        player.Platform = PlatformFromAuthType(_ludus->LocalAuthType());
        player.GameVersion = Core::SnoreSaberRuntimeInfo::GameVersion();
        player.ClientVersion = Core::SnoreSaberRuntimeInfo::PluginVersion();
        player.ReplayModVersion = Core::SnoreSaberRuntimeInfo::PluginVersion();
        return player;
    }

    V1::BeatmapIdentity LiveReplayStreamingService::BeatmapIdentity()
    {
        V1::BeatmapIdentity beatmap{};
        beatmap.MapHash = ExtractMapHash(_metadata->LevelID);
        beatmap.LevelId = _metadata->LevelID;
        beatmap.Difficulty = _metadata->Difficulty;
        beatmap.DifficultyName = DifficultyName(_metadata->Difficulty);
        beatmap.Characteristic = _metadata->Characteristic;
        beatmap.LeaderboardId = ExtractLeaderboardId(_metadata->LevelID);
        beatmap.Modifiers = _metadata->Modifiers;
        beatmap.MaxScore = _lastMaxScore;
        return beatmap;
    }

    V1::StreamReplayMetadata LiveReplayStreamingService::StreamMetadata()
    {
        V1::StreamReplayMetadata metadata{};
        metadata.ReplayVersion = _metadata->Version.str();
        metadata.LevelId = _metadata->LevelID;
        metadata.Difficulty = _metadata->Difficulty;
        metadata.Characteristic = _metadata->Characteristic;
        metadata.Environment = _metadata->Environment;
        metadata.Modifiers = _metadata->Modifiers;
        metadata.NoteSpawnOffset = _metadata->NoteSpawnOffset;
        metadata.LeftHanded = _metadata->LeftHanded;
        metadata.InitialHeight = _metadata->InitialHeight;
        metadata.RoomRotation = _metadata->RoomRotation;
        metadata.RoomCenter = ToReplayVector(_metadata->RoomCenter);
        metadata.FailTimeSeconds = _metadata->FailTime;
        metadata.GameVersion = _metadata->GameVersion.has_value() ? _metadata->GameVersion->str() : "";
        metadata.PluginVersion = _metadata->PluginVersion.has_value() ? _metadata->PluginVersion->str() : "";
        metadata.Platform = _metadata->Platform.value_or("");
        metadata.SongSpeed = _metadata->SongSpeed > 0.0f ? _metadata->SongSpeed : 1.0f;
        metadata.JumpDistance = _metadata->JumpDistance;
        metadata.LeftSaberColor = ToReplayColor(_metadata->LeftSaberColor);
        metadata.RightSaberColor = ToReplayColor(_metadata->RightSaberColor);
        return metadata;
    }

    V1::ReplayScoreSummary LiveReplayStreamingService::ScoreSummary(GlobalNamespace::LevelCompletionResults* results)
    {
        double accuracy = 0.0;
        if (_lastMaxScore > 0)
        {
            accuracy = std::clamp(static_cast<double>(results->modifiedScore) / _lastMaxScore, 0.0, 1.0);
        }

        V1::ReplayScoreSummary summary{};
        summary.Score = ToUint(results->multipliedScore);
        summary.ModifiedScore = ToUint(results->modifiedScore);
        summary.MaxScore = _lastMaxScore;
        summary.Accuracy = accuracy;
        summary.Combo = ToUint(results->maxCombo);
        summary.MaxCombo = ToUint(results->maxCombo);
        summary.FullCombo = results->fullCombo;
        summary.GoodCuts = ToUint(results->goodCutsCount);
        summary.BadCuts = ToUint(results->badCutsCount);
        summary.MissedNotes = ToUint(results->missedCount);
        return summary;
    }

    bool LiveReplayStreamingService::CanUsePublicPresenceForCurrentLevel() const
    {
        if (!_metadata)
        {
            return false;
        }

        const std::string& levelId = _metadata->LevelID;
        return Utils::SnoreSaberBeatmapKey::IsCustomLevelId(levelId) &&
               !Utils::SnoreSaberBeatmapKey::IsWipLevelId(levelId) &&
               Utils::SnoreSaberBeatmapKey::TryGetSongHash(levelId).has_value();
    }

    int64_t LiveReplayStreamingService::UnixNowMs()
    {
        return _clock->UnixTimeMilliseconds();
    }

    std::vector<V1::ReplayExtension> LiveReplayStreamingService::ToReplayExtensions(const std::vector<Data::Private::ReplayExtensionPayloads::ReplayExtensionEntry>& entries)
    {
        std::vector<V1::ReplayExtension> extensions;
        extensions.reserve(entries.size());
        for (const auto& entry : entries)
        {
            extensions.push_back(ToReplayExtension(entry));
        }
        return extensions;
    }

    V1::ReplayExtension LiveReplayStreamingService::ToReplayExtension(const Data::Private::ReplayExtensionPayloads::ReplayExtensionEntry& entry)
    {
        V1::ReplayExtension extension{};
        extension.Id = entry.Id;
        extension.Version = entry.Version > 0 ? static_cast<uint32_t>(entry.Version) : 0;
        extension.Payload.assign(entry.Payload.begin(), entry.Payload.end());
        return extension;
    }

    V1::ReplayPoseFrame LiveReplayStreamingService::ToReplayPoseFrame(const Data::Private::VRPoseGroup& frame)
    {
        V1::ReplayPoseFrame poseFrame{};
        poseFrame.Head = ToReplayPose(frame.Head);
        poseFrame.Left = ToReplayPose(frame.Left);
        poseFrame.Right = ToReplayPose(frame.Right);
        poseFrame.Fps = frame.FPS;
        poseFrame.TimeSeconds = frame.Time;
        return poseFrame;
    }

    V1::ReplayPose LiveReplayStreamingService::ToReplayPose(const Data::Private::VRPose& pose)
    {
        V1::ReplayPose replayPose{};
        replayPose.Position = ToReplayVector(pose.Position);
        replayPose.Rotation = ToReplayQuaternion(pose.Rotation);
        return replayPose;
    }

    V1::ReplayNoteEvent LiveReplayStreamingService::ToReplayNoteEvent(const Data::Private::NoteEvent& note)
    {
        V1::ReplayNoteId noteId{};
        noteId.TimeSeconds = note.TheNoteID.Time;
        noteId.LineLayer = note.TheNoteID.LineLayer;
        noteId.LineIndex = note.TheNoteID.LineIndex;
        noteId.ColorType = note.TheNoteID.ColorType;
        noteId.CutDirection = note.TheNoteID.CutDirection;
        if (note.TheNoteID.GameplayType.has_value())
        {
            noteId.GameplayType = *note.TheNoteID.GameplayType;
        }
        if (note.TheNoteID.ScoringType.has_value())
        {
            noteId.ScoringType = *note.TheNoteID.ScoringType;
        }
        if (note.TheNoteID.CutDirectionAngleOffset.has_value())
        {
            noteId.CutDirectionAngleOffset = *note.TheNoteID.CutDirectionAngleOffset;
        }

        V1::ReplayNoteEvent noteEvent{};
        noteEvent.NoteId = noteId;
        noteEvent.EventType = ToReplayNoteEventType(note.EventType);
        noteEvent.CutPoint = ToReplayVector(note.CutPoint);
        noteEvent.CutNormal = ToReplayVector(note.CutNormal);
        noteEvent.SaberDirection = ToReplayVector(note.SaberDirection);
        noteEvent.SaberType = note.SaberType;
        noteEvent.DirectionOk = note.DirectionOK;
        noteEvent.SaberSpeed = note.SaberSpeed;
        noteEvent.CutAngle = note.CutAngle;
        noteEvent.CutDistanceToCenter = note.CutDistanceToCenter;
        noteEvent.CutDirectionDeviation = note.CutDirectionDeviation;
        noteEvent.BeforeCutRating = note.BeforeCutRating;
        noteEvent.AfterCutRating = note.AfterCutRating;
        noteEvent.TimeSeconds = note.Time;
        noteEvent.UnityTimescale = note.UnityTimescale;
        noteEvent.TimeSyncTimescale = note.TimeSyncTimescale;
        if (note.TimeDeviation.has_value())
        {
            noteEvent.TimeDeviation = *note.TimeDeviation;
        }
        noteEvent.WorldRotation = ToReplayQuaternion(note.WorldRotation);
        noteEvent.InverseWorldRotation = ToReplayQuaternion(note.InverseWorldRotation);
        noteEvent.NoteRotation = ToReplayQuaternion(note.NoteRotation);
        noteEvent.NotePosition = ToReplayVector(note.NotePosition);
        return noteEvent;
    }

    V1::ReplayVector3 LiveReplayStreamingService::ToReplayVector(const Data::Private::VRPosition& position)
    {
        return V1::ReplayVector3{.X = position.X, .Y = position.Y, .Z = position.Z};
    }

    V1::ReplayVector3 LiveReplayStreamingService::ToReplayVector(const std::optional<Data::Private::VRPosition>& position)
    {
        return position.has_value() ? ToReplayVector(*position) : V1::ReplayVector3{};
    }

    V1::ReplayQuaternion LiveReplayStreamingService::ToReplayQuaternion(const Data::Private::VRRotation& rotation)
    {
        return V1::ReplayQuaternion{.X = rotation.X, .Y = rotation.Y, .Z = rotation.Z, .W = rotation.W};
    }

    V1::ReplayQuaternion LiveReplayStreamingService::ToReplayQuaternion(const std::optional<Data::Private::VRRotation>& rotation)
    {
        return rotation.has_value() ? ToReplayQuaternion(*rotation) : V1::ReplayQuaternion{};
    }

    std::optional<V1::ReplayColor> LiveReplayStreamingService::ToReplayColor(const std::optional<UnityEngine::Color>& color)
    {
        if (!color.has_value())
        {
            return std::nullopt;
        }

        return V1::ReplayColor{.R = color->r, .G = color->g, .B = color->b, .A = color->a};
    }

    V1::ReplayNoteEventType LiveReplayStreamingService::ToReplayNoteEventType(Data::Private::NoteEventType type)
    {
        switch (type)
        {
            case Data::Private::NoteEventType::GoodCut:
                return V1::ReplayNoteEventType::GoodCut;
            case Data::Private::NoteEventType::BadCut:
                return V1::ReplayNoteEventType::BadCut;
            case Data::Private::NoteEventType::Miss:
                return V1::ReplayNoteEventType::Miss;
            case Data::Private::NoteEventType::Bomb:
                return V1::ReplayNoteEventType::Bomb;
            default:
                return V1::ReplayNoteEventType::Unspecified;
        }
    }

    V1::ReplayCompletion LiveReplayStreamingService::CompletionFromResults(GlobalNamespace::LevelCompletionResults* results, std::optional<Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride)
    {
        if (playOutcomeOverride.has_value())
        {
            return CompletionFromPlayOutcome(*playOutcomeOverride);
        }

        if (results->levelEndAction == GlobalNamespace::LevelCompletionResults::LevelEndAction::Quit)
        {
            return V1::ReplayCompletion::Quit;
        }

        if (results->levelEndAction == GlobalNamespace::LevelCompletionResults::LevelEndAction::Restart)
        {
            return V1::ReplayCompletion::Aborted;
        }

        if (results->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Failed)
        {
            return V1::ReplayCompletion::Failed;
        }

        if (results->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Cleared)
        {
            return V1::ReplayCompletion::Passed;
        }

        return V1::ReplayCompletion::Aborted;
    }

    V1::ReplayCompletion LiveReplayStreamingService::CompletionFromPlayOutcome(Core::Gameplay::SnoreSaberPlayOutcome outcome)
    {
        switch (outcome)
        {
            case Core::Gameplay::SnoreSaberPlayOutcome::Clear:
                return V1::ReplayCompletion::Passed;
            case Core::Gameplay::SnoreSaberPlayOutcome::Fail:
                return V1::ReplayCompletion::Failed;
            case Core::Gameplay::SnoreSaberPlayOutcome::Quit:
                return V1::ReplayCompletion::Quit;
            case Core::Gameplay::SnoreSaberPlayOutcome::Restart:
                return V1::ReplayCompletion::Aborted;
            default:
                return V1::ReplayCompletion::Unspecified;
        }
    }

    V1::ReplayPlatform LiveReplayStreamingService::PlatformFromAuthType(int authType)
    {
        // quest auth types: 2 = quest, 3 = dev
        switch (authType)
        {
            case 2:
                return V1::ReplayPlatform::MetaQuest;
            case 3:
                return V1::ReplayPlatform::Dev;
            default:
                return V1::ReplayPlatform::Unspecified;
        }
    }

    std::string LiveReplayStreamingService::ExtractMapHash(const std::string& levelId)
    {
        return Utils::SnoreSaberBeatmapKey::TryGetSongHash(levelId).value_or("");
    }

    std::string LiveReplayStreamingService::ExtractLeaderboardId(const std::string& levelId)
    {
        return ExtractMapHash(levelId);
    }

    std::string LiveReplayStreamingService::DifficultyName(int difficulty)
    {
        switch (difficulty)
        {
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
                return std::to_string(difficulty);
        }
    }

    V1::ReplayEventCounts LiveReplayStreamingService::CloneCounts(const V1::ReplayEventCounts& counts)
    {
        return counts;
    }

    uint32_t LiveReplayStreamingService::ToUint(int value)
    {
        return value > 0 ? static_cast<uint32_t>(value) : 0;
    }
}
