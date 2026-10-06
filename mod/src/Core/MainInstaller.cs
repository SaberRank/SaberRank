using SaberRank.Core.Api;
using SaberRank.Core.BeatSaver;
using SaberRank.Core.Presentation;
using SaberRank.Features.Leaderboards;
using SaberRank.Features.Live;
using SaberRank.Features.MainMenu;
using SaberRank.Features.Players;
using SaberRank.Features.Replays;
using SaberRank.Features.ScoreSubmission;
using SaberRank.Features.Multiplayer;
using Zenject;

namespace SaberRank.Core {
    internal partial class MainInstaller : Installer {

        public override void InstallBindings() {
            Container.BindInstance(new object()).WithId("SaberRankUIBindings").AsCached();
            Container.Bind<ISaberRankApiClient>().To<SaberRankApiClient>().AsSingle();
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
