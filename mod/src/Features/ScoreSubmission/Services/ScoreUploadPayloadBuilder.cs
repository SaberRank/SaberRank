using Newtonsoft.Json;
using SnoreSaber.Core;
using SnoreSaber.Core.Gameplay;
using SnoreSaber.Features.Players.Domain;
using SnoreSaber.Features.ScoreSubmission.Domain;
using System;

namespace SnoreSaber.Features.ScoreSubmission.Services {

    internal class ScoreUploadPayloadBuilder {
        private const string UploadSecret = "f0b4a81c9bd3ded1081b365f7628781f";
        private readonly SnoreSaberRuntimeInfo _runtimeInfo;

        public ScoreUploadPayloadBuilder(SnoreSaberRuntimeInfo runtimeInfo) {
            _runtimeInfo = runtimeInfo;
        }

        internal ScoreUploadPayload Build(BeatmapLevel beatmapLevel, BeatmapKey beatmapKey, LevelCompletionResults results, LocalPlayerInfo playerInfo, float playOutcomeTime, SnoreSaberPlayOutcome? playOutcomeOverride) {
            SnoreSaberUploadData scoreData = SnoreSaberUploadData.Create(beatmapLevel, beatmapKey, results, playerInfo, _runtimeInfo.UploadVersionHash, playOutcomeTime, playOutcomeOverride);
            string serializedScore = JsonConvert.SerializeObject(scoreData);
            return new ScoreUploadPayload {
                ScoreData = scoreData,
                // SnoreSaber owns this upload endpoint. Keep the payload as JSON so the
                // SnoreSaber server can validate it directly; do not use the legacy
                // ScoreSaber encrypted upload format.
                EncryptedScoreData = serializedScore
            };
        }

    }

    internal class ScoreUploadPayload {
        internal SnoreSaberUploadData ScoreData { get; set; }
        internal string EncryptedScoreData { get; set; } = string.Empty;
    }
}
