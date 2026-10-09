using SnoreSaber.Core.Api;
using SnoreSaber.Core.BeatSaver;
using SnoreSaber.Core.Presentation;
using SnoreSaber.Features.Leaderboards;
using SnoreSaber.Features.Live;
using SnoreSaber.Features.MainMenu;
using SnoreSaber.Features.Players;
using SnoreSaber.Features.Replays;
using SnoreSaber.Features.ScoreSubmission;
using SnoreSaber.Features.Multiplayer;
using Zenject;

namespace SnoreSaber.Core {
    internal partial class MainInstaller : Installer {

        public override void InstallBindings() {
            Container.BindInstance(new object()).WithId("SnoreSaberUIBindings").AsCached();
            Container.Bind<ISnoreSaberApiClient>().To<SnoreSaberApiClient>().AsSingle();
            Container.Bind<BeatSaverService>().AsSingle();
            Container.BindInterfacesAndSelfTo<RemoteImageService>().AsSingle();
            InstallGameBindings();
            Container.Install<PlayersFeatureInstaller>();
            Container.Install<ReplayFeatureInstaller>();
            Container.Install<ScoreSubmissionFeatureInstaller>();
            Container.Install<LeaderboardFeatureInstaller>();
            Container.Install<LiveFeatureInstaller>();
            Container.Install<MainMenuFeatureInstaller>();
            Container.BindInterfacesTo<MultiplayerSessionController>().AsSingle();
            Container.BindInterfacesTo<MultiplayerLeaderboardController>().AsSingle();
        }

        partial void InstallGameBindings();
    }
}
