using SaberRank.Core.Api;
using SaberRank.Features.Leaderboards.Domain;
using SaberRank.Features.Replays;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace SaberRank.Features.Replays.Services {
    internal class ReplayQueryService {
        private readonly ISaberRankApiClient _apiClient;
        private readonly ReplayStorageService _replayStorageService;

        public ReplayQueryService(ISaberRankApiClient apiClient, ReplayStorageService replayStorageService) {
            _apiClient = apiClient;
            _replayStorageService = replayStorageService;
        }

        public async Task<byte[]> GetReplayData(ScoreMap scoreMap) {
            Exception downloadError = null;
            try {
                byte[] response = await _apiClient.DownloadReplay(scoreMap.Score.Id, CancellationToken.None);
                if (response != null) {
                    return response;
                }
            } catch (Exception ex) {
                downloadError = ex;
                Plugin.Log.Debug($"Failed to download SaberRank replay, checking local fallback: {ex.Message}");
            }

            if (scoreMap.HasLocalReplay) {
                byte[] replay = _replayStorageService.ReadLocalReplay(scoreMap.Parent.BeatmapLevel, scoreMap.Parent.BeatmapKey, scoreMap);
                if (replay != null) {
                    return replay;
                }
            }

            throw new Exception("Failed to download replay", downloadError);
        }
    }
}
