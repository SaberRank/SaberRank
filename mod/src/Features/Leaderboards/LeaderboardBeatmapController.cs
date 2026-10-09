using LeaderboardCore.Interfaces;
using SnoreSaber.Features.Leaderboards.Domain;
using SnoreSaber.Features.Leaderboards.Services;
using SnoreSaber.Features.Leaderboards.UI;
using SnoreSaber.Features.Leaderboards.UI.Avatars;
using SnoreSaber.Features.Players.Services;

namespace SnoreSaber.Features.Leaderboards {
    internal partial class LeaderboardBeatmapController : INotifyLeaderboardSet {
        private readonly SnoreSaberLeaderboardOverlayController _overlayController;
        private readonly LeaderboardAvatarHost _avatarHost;
        private readonly GameSessionService _gameSessionService;
        private readonly LeaderboardScreenSession _leaderboardSession;

        public LeaderboardBeatmapController(
            SnoreSaberLeaderboardOverlayController overlayController,
            LeaderboardAvatarHost avatarHost,
            GameSessionService gameSessionService,
            LeaderboardScreenSession leaderboardSession) {
            _overlayController = overlayController;
            _avatarHost = avatarHost;
            _gameSessionService = gameSessionService;
            _leaderboardSession = leaderboardSession;
        }

        public void OnLeaderboardSet(BeatmapKey beatmapKey) {
            if (!SnoreSaberBeatmapKey.IsSupported(beatmapKey)) {
                _leaderboardSession.ClearBeatmap();
                return;
            }

            bool parsed = _overlayController.IsParsed;
            _overlayController.EnsureParsed();
            if (!parsed) {
                _avatarHost.ClearAvatars();
            }

            if (!SnoreSaberBeatmapKey.IsWip(beatmapKey)) {
                _gameSessionService.EnsureAuthenticated();
            }
            _leaderboardSession.SetBeatmap(beatmapKey);
        }
    }
}
