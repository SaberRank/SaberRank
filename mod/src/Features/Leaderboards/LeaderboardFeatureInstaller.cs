using SaberRank.Features.Leaderboards.Adapters.LeaderboardCore;
using SaberRank.Features.Leaderboards.Services;
using SaberRank.Features.Leaderboards.UI;
using SaberRank.Features.Leaderboards.UI.Avatars;
using Zenject;

namespace SaberRank.Features.Leaderboards {
    internal class LeaderboardFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.Bind<BeatmapMaxScoreCache>().AsSingle();
            Container.Bind<LeaderboardPlayerScoreCache>().AsSingle();
            Container.Bind<LeaderboardScreenLoader>().AsSingle();
            Container.BindInterfacesAndSelfTo<LeaderboardScreenSession>().AsSingle();
            Container.Bind<LeaderboardTweeningService>().AsSingle();
            Container.Bind<LeaderboardQueryService>().AsSingle();
            Container.Bind<LeaderboardAvatarHost>().AsSingle();
            Container.BindInterfacesTo<LeaderboardPanelFlow>().AsSingle().NonLazy();
            Container.BindInterfacesTo<LeaderboardBeatmapController>().AsSingle().NonLazy();
            Container.BindInterfacesTo<LeaderboardInteractionController>().AsSingle().NonLazy();
            Container.BindInterfacesTo<LeaderboardPresentationController>().AsSingle().NonLazy();
            Container.BindInterfacesTo<LeaderboardStatusController>().AsSingle().NonLazy();
            Container.BindInterfacesAndSelfTo<LeaderboardModalFlow>().AsSingle();

            Container.Bind<PanelView>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<SaberRankLeaderboardCoreViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<SaberRankLeaderboardOverlayController>().AsSingle().NonLazy();

            Container.BindInterfacesAndSelfTo<SaberRankCustomLeaderboard>().AsSingle();
        }
    }
}
