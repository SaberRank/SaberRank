using Newtonsoft.Json;
using SnoreSaber.Core.Gameplay;
using SnoreSaber.Core.Platform;
using SnoreSaber.Features.Leaderboards.Domain;
using SnoreSaber.Features.Players.Domain;
using System;
using System.Collections.Generic;
using System.Linq;

namespace SnoreSaber.Features.ScoreSubmission.Domain {

    internal class SnoreSaberUploadData {
        [JsonProperty("playerName")]
        internal string PlayerName { get; set; }
        [JsonProperty("playerId")]
        internal string PlayerId { get; set; }
        [JsonProperty("score")]
        internal int Score { get; set; }
        [JsonProperty("leaderboardId")]
        internal string LeaderboardId { get; set; }
        [JsonProperty("songName")]
        internal string SongName { get; set; }
        [JsonProperty("songSubName")]
        internal string SongSubName { get; set; }
        [JsonProperty("levelAuthorName")]
        internal string LevelAuthorName { get; set; }
        [JsonProperty("songAuthorName")]
        internal string SongAuthorName { get; set; }
        [JsonProperty("bpm")]
        internal int BPM { get; set; }
        [JsonProperty("difficulty")]
        internal int Difficulty { get; set; }
        [JsonProperty("infoHash")]
        internal string InfoHash { get; set; }
        [JsonProperty("modifiers")]
        internal List<string> Modifiers { get; set; }
        [JsonProperty("gameMode")]
        internal string GameMode { get; set; }
        [JsonProperty("playOutcome")]
        [JsonConverter(typeof(Newtonsoft.Json.Converters.StringEnumConverter))]
        internal SnoreSaberPlayOutcome PlayOutcome { get; set; }
        [JsonProperty("playOutcomeTime")]
        internal float PlayOutcomeTime { get; set; }
        [JsonProperty("badCutsCount")]
        internal int BadCutsCount { get; set; }
        [JsonProperty("missedCount")]
        internal int MissedCount { get; set; }
        [JsonProperty("maxCombo")]
        internal int MaxCombo { get; set; }
        [JsonProperty("fullCombo")]
        internal bool FullCombo { get; set; }
        [JsonProperty("hmd")]
        internal int? HMD { get; set; }
        [JsonProperty("deviceHmdIdentifier")]
        internal string DeviceHMDIdentifier { get; set; }
        [JsonProperty("deviceControllerLeftIdentifier")]
        internal string DeviceControllerLeftIdentifier { get; set; }
        [JsonProperty("deviceControllerRightIdentifier")]
        internal string DeviceControllerRightIdentifier { get; set; }

        internal static SnoreSaberUploadData Create(BeatmapLevel beatmapLevel, BeatmapKey beatmapKey, LevelCompletionResults results, LocalPlayerInfo playerInfo, string infoHash, float playOutcomeTime, SnoreSaberPlayOutcome? playOutcomeOverride) {
            return new SnoreSaberUploadData {
                GameMode = $"Solo{beatmapKey.CharacteristicSerializedName()}",
                Difficulty = BeatmapDifficultyMethods.DefaultRating(beatmapKey.difficulty),
                InfoHash = SnoreSaberBeatmapKey.GetSongHash(beatmapKey),
                LeaderboardId = SnoreSaberBeatmapKey.GetSongHash(beatmapKey),
                SongName = beatmapLevel.songName,
                SongSubName = beatmapLevel.songSubName,
                SongAuthorName = beatmapLevel.songAuthorName,
                LevelAuthorName = FriendlyLevelAuthorName(beatmapLevel.allMappers, beatmapLevel.allLighters),
                BPM = Convert.ToInt32(beatmapLevel.beatsPerMinute),
                PlayerName = playerInfo.playerName,
                PlayerId = playerInfo.playerId,
                BadCutsCount = results.badCutsCount,
                MissedCount = results.missedCount,
                MaxCombo = results.maxCombo,
                FullCombo = results.fullCombo,
                Score = results.multipliedScore,
                Modifiers = SnoreSaberGameplayModifiers.ToCodeList(results),
                PlayOutcome = playOutcomeOverride ?? SnoreSaberPlayOutcomes.FromLevelCompletionResults(results),
                PlayOutcomeTime = playOutcomeTime,
                HMD = null,
                DeviceHMDIdentifier = VRDevices.GetDeviceHMD(),
                DeviceControllerLeftIdentifier = VRDevices.GetDeviceControllerLeft(),
                DeviceControllerRightIdentifier = VRDevices.GetDeviceControllerRight()
            };
        }

        private static string FriendlyLevelAuthorName(string[] mappers, string[] lighters) {
            List<string> mappersAndLighters = new List<string>();
            mappersAndLighters.AddRange(mappers);
            mappersAndLighters.AddRange(lighters);

            if (mappersAndLighters.Count == 0) {
                return string.Empty;
            }
            if (mappersAndLighters.Count == 1) {
                return mappersAndLighters.First();
            }
            return $"{string.Join(", ", mappersAndLighters.Take(mappersAndLighters.Count - 1))} & {mappersAndLighters.Last()}";
        }
    }
}
