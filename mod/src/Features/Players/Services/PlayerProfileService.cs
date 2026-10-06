using SaberRank.Core.Api;
using SaberRank.Features.Players.Domain;
using System.Threading;
using System.Threading.Tasks;

namespace SaberRank.Features.Players.Services {
    internal class PlayerProfileService {
        private readonly ISaberRankApiClient _apiClient;

        public PlayerProfileService(ISaberRankApiClient apiClient) {
            _apiClient = apiClient;
        }

        public async Task<PlayerProfile> GetPlayerInfo(string playerId, bool full) {
            PlayerProfile player = await _apiClient.GetPlayerProfile(playerId, full, null, CancellationToken.None);
            if (full) {
                player.GlobalHistory = await _apiClient.GetGlobalPlayerHistory(playerId, CancellationToken.None);
            }
            return player;
        }
    }
}
