using LeaderboardCore.Interfaces;
using SaberRank.Features.Leaderboards.Domain;
using SaberRank.Features.Leaderboards.Services;
using SaberRank.Features.Leaderboards.UI;
using SaberRank.Features.Leaderboards.UI.Avatars;
using SaberRank.Features.Players.Services;

namespace SaberRank.Features.Leaderboards {
    internal partial class LeaderboardBeatmapController : INotifyLeaderboardSet {
        private readonly SaberRankLeaderboardOverlayController _overlayController;
        private readonly LeaderboardAvatarHost _avatarHost;
        private readonly GameSessionService _gameSessionService;
        private readonly LeaderboardScreenSession _leaderboardSession;

        public LeaderboardBeatmapController(
            SaberRankLeaderboardOverlayController overlayController,
            LeaderboardAvatarHost avatarHost,
            GameSessionService gameSessionService,
            LeaderboardScreenSession leaderboardSession) {
            _overlayController = overlayController;
            _avatarHost = avatarHost;
            _gameSessionService = gameSessionService;
            _leaderboardSession = leaderboardSession;
        }

        public void OnLeaderboardSet(BeatmapKey beatmapKey) {
            if (!SaberRankBeatmapKey.IsSupported(beatmapKey)) {
                _leaderboardSession.ClearBeatmap();
                return;
            }

            bool parsed = _overlayController.IsParsed;
            _overlayController.EnsureParsed();
            if (!parsed) {
                _avatarHost.ClearAvatars();
            }

            if (!SaberRankBeatmapKey.IsWip(beatmapKey)) {
                _gameSessionService.EnsureAuthenticated();
            }
            _leaderboardSession.SetBeatmap(beatmapKey);
        }
    }
}
