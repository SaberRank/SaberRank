using HMUI;
using LeaderboardCore.Managers;
using SnoreSaber.Features.Leaderboards.Domain;
using SnoreSaber.Features.Leaderboards.UI;
using System;
using Zenject;

namespace SnoreSaber.Features.Leaderboards.Adapters.LeaderboardCore {

    internal class SnoreSaberCustomLeaderboard : CustomLeaderboardAdapter, IInitializable, IDisposable {
        private readonly CustomLeaderboardManager _customLeaderboardManager;
        private readonly SnoreSaberLeaderboardCoreViewController _leaderboardViewController;
        private readonly PanelView _panelView;

        public SnoreSaberCustomLeaderboard(CustomLeaderboardManager customLeaderboardManager, SnoreSaberLeaderboardCoreViewController leaderboardViewController, PanelView panelView) {
            _customLeaderboardManager = customLeaderboardManager;
            _leaderboardViewController = leaderboardViewController;
            _panelView = panelView;
        }

        protected override string leaderboardId => "SnoreSaber";

        protected override ViewController leaderboardViewController => _leaderboardViewController;

        protected override ViewController panelViewController => _panelView;

        protected override bool ShowForLevelId(string levelId) => SnoreSaberBeatmapKey.IsSupportedLevelId(levelId);

        public void Initialize() {
            Plugin.Log.Info("Registering SnoreSaber custom leaderboard with LeaderboardCore.");
            _customLeaderboardManager.Register(this);
            Plugin.Log.Info("SnoreSaber custom leaderboard registered with LeaderboardCore.");
        }

        public void Dispose() {
            try {
                _customLeaderboardManager.Unregister(this);
            } catch (Exception ex) {
                Plugin.Log.Warn($"Failed to unregister SnoreSaber custom leaderboard: {ex.Message}");
            }
        }
    }
}
