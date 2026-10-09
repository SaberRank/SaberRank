using SnoreSaber.Features.Leaderboards.Adapters.LeaderboardCore;
using SnoreSaber.Features.Leaderboards.Services;
using SnoreSaber.Features.Leaderboards.UI;
using SnoreSaber.Features.Leaderboards.UI.Avatars;
using Zenject;

namespace SnoreSaber.Features.Leaderboards {
    internal class LeaderboardFeatureInstaller : Installer {
        public override void InstallBindings() {
            Plugin.Log.Info("Installing SnoreSaber leaderboard feature bindings.");
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
            Container.Bind<SnoreSaberLeaderboardCoreViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<SnoreSaberLeaderboardOverlayController>().AsSingle().NonLazy();

            Container.BindInterfacesAndSelfTo<SnoreSaberCustomLeaderboard>().AsSingle();
        }
    }
}
