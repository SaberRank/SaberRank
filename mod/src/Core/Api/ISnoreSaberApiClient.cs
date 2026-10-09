using SnoreSaber.Core.Api.Paging;
using SnoreSaber.Core.Api.Generated;
using SnoreSaber.Features.Players.Domain;
using SnoreSaber.Features.Leaderboards.Domain;
using SnoreSaber.Features.ScoreSubmission.Domain;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace SnoreSaber.Core.Api {

    internal interface ISnoreSaberApiClient {
        Task<GameAuthenticationResult> AuthenticateGame(GameAuthenticationRequest request, CancellationToken cancellationToken);
        Task<ScoreUploadResult> UploadScore(GameSession session, string uploadData, string uploadVersionHash, byte[] replay, CancellationToken cancellationToken);
        Task<LeaderboardSnapshot> GetLeaderboard(LeaderboardQuery query, GameSession session, CancellationToken cancellationToken);
        Task<LeaderboardDetails> GetLeaderboardDetails(LeaderboardQuery query, CancellationToken cancellationToken);
        Task<MapDetailsResponse> GetMapById(int mapId, CancellationToken cancellationToken);
        Task<MapDetailsResponse> GetMapByHash(string hash, CancellationToken cancellationToken);
        Task<PagedResult<PlayerSummary>> GetPlayers(PlayerListQuery query, GameSession session, CancellationToken cancellationToken);
        Task<PlayerProfile> GetPlayerProfile(string playerId, bool full, int? realmId, CancellationToken cancellationToken);
        Task<List<PlayerHistoryPoint>> GetGlobalPlayerHistory(string playerId, CancellationToken cancellationToken);
        Task<byte[]> DownloadReplay(int scoreId, CancellationToken cancellationToken);
        Task<List<LivePlayerTournamentSummary>> ListLivePlayerTournaments(GameSession session, CancellationToken cancellationToken);
        Task<List<LivePlayerRoomSummary>> ListLivePlayerRooms(string tournamentId, GameSession session, CancellationToken cancellationToken);
        Task<LivePlayerRoomDetails> GetLivePlayerRoom(string tournamentId, string matchId, GameSession session, CancellationToken cancellationToken);
        Task<LivePlayerRoomDetails> GetLivePlayerRoomByInviteCode(string inviteCode, GameSession session, CancellationToken cancellationToken);
    }
}
