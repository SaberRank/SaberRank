using HMUI;
using LeaderboardCore.Managers;
using SaberRank.Features.Leaderboards.Domain;
using SaberRank.Features.Leaderboards.UI;
using System;
using Zenject;

namespace SaberRank.Features.Leaderboards.Adapters.LeaderboardCore {

    internal class SaberRankCustomLeaderboard : CustomLeaderboardAdapter, IInitializable, IDisposable {
        private readonly CustomLeaderboardManager _customLeaderboardManager;
        private readonly SaberRankLeaderboardCoreViewController _leaderboardViewController;
        private readonly PanelView _panelView;

        public SaberRankCustomLeaderboard(CustomLeaderboardManager customLeaderboardManager, SaberRankLeaderboardCoreViewController leaderboardViewController, PanelView panelView) {
            _customLeaderboardManager = customLeaderboardManager;
            _leaderboardViewController = leaderboardViewController;
            _panelView = panelView;
        }

        protected override string leaderboardId => "SaberRank";

        protected override ViewController leaderboardViewController => _leaderboardViewController;

        protected override ViewController panelViewController => _panelView;

        protected override bool ShowForLevelId(string levelId) => SaberRankBeatmapKey.IsSupportedLevelId(levelId);

        public void Initialize() => _customLeaderboardManager.Register(this);

        public void Dispose() => _customLeaderboardManager.Unregister(this);
    }
}
