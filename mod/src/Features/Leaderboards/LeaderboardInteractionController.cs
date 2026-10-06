using SaberRank.Features.Leaderboards.Adapters.LeaderboardCore;
using SaberRank.Features.Leaderboards.Domain;
using SaberRank.Features.Leaderboards.Services;
using SaberRank.Features.Leaderboards.UI;
using System;
using Zenject;

namespace SaberRank.Features.Leaderboards {
    internal class LeaderboardInteractionController : IInitializable, IDisposable {
        private readonly SaberRankLeaderboardCoreViewController _leaderboardViewController;
        private readonly LeaderboardScreenSession _leaderboardSession;
        private readonly LeaderboardModalFlow _modalFlow;

        public LeaderboardInteractionController(
            SaberRankLeaderboardCoreViewController leaderboardViewController,
            LeaderboardScreenSession leaderboardSession,
            LeaderboardModalFlow modalFlow) {
            _leaderboardViewController = leaderboardViewController;
            _leaderboardSession = leaderboardSession;
            _modalFlow = modalFlow;
        }

        public void Initialize() {
            _leaderboardViewController.ScoreSelected += LeaderboardViewControllerScoreSelected;
            _leaderboardViewController.ScopeSelected += LeaderboardViewControllerScopeSelected;
            _leaderboardViewController.PageUpRequested += LeaderboardViewControllerPageUpRequested;
            _leaderboardViewController.PageDownRequested += LeaderboardViewControllerPageDownRequested;
        }

        private void LeaderboardViewControllerScoreSelected(int index) {
            ScoreMap score = _leaderboardSession.GetScore(index);
            if (score == null) {
                return;
            }

            _modalFlow.ShowScore(score);
        }

        private void LeaderboardViewControllerScopeSelected(LeaderboardScreenScope scope) => _leaderboardSession.SelectScope(scope);

        private void LeaderboardViewControllerPageUpRequested() => _leaderboardSession.PageUp();

        private void LeaderboardViewControllerPageDownRequested() => _leaderboardSession.PageDown();

        public void Dispose() {
            _leaderboardViewController.ScoreSelected -= LeaderboardViewControllerScoreSelected;
            _leaderboardViewController.ScopeSelected -= LeaderboardViewControllerScopeSelected;
            _leaderboardViewController.PageUpRequested -= LeaderboardViewControllerPageUpRequested;
            _leaderboardViewController.PageDownRequested -= LeaderboardViewControllerPageDownRequested;
        }
    }
}
