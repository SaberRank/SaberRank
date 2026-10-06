using SaberRank.Features.Live.Ludus.Services;
using SaberRank.Features.Live.Compete.Services;
using SaberRank.Features.Live.Compete.UI.FlowCoordinators;
using SaberRank.Features.Live.Compete.UI.ViewControllers.CodeEntry;
using SaberRank.Features.Live.Compete.UI.ViewControllers.Entry;
using SaberRank.Features.Live.Compete.UI.ViewControllers.Room.Center;
using SaberRank.Features.Live.Compete.UI.ViewControllers.Room.Left;
using SaberRank.Features.Live.Compete.UI.ViewControllers.Rooms;
using SaberRank.Features.Live.Compete.UI.ViewControllers.Shared;
using SaberRank.Features.Live.Ludus.UI;
using SaberRank.Features.Live.UI.ViewControllers;
using Zenject;

namespace SaberRank.Features.Live {
    internal class LiveFeatureInstaller : Installer {
        public override void InstallBindings() {
            Container.Bind<CompeteModeSelectionViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<TournamentBrowserViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<CompeteRoomListViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<CompeteRoomViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<CompetePlayerListViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<LiveChatFloatingViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<CompeteCodeEntryViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<CompeteLoadingViewController>().FromNewComponentAsViewController().AsSingle();
            Container.Bind<CompeteDirectoryService>().AsSingle();
            Container.Bind<CompeteSongService>().AsSingle();
            Container.Bind<LiveChatSongNavigator>().AsSingle();
            Container.Bind<LiveChatLinkService>().AsSingle();
            Container.Bind<CompeteGameplayLauncher>().AsSingle();
            Container.BindInterfacesAndSelfTo<LudusSessionService>().AsSingle().NonLazy();
            Container.BindInterfacesTo<LiveChatOverlayController>().AsSingle().NonLazy();
            Container.Bind<CompeteFlowCoordinator>().FromNewComponentOnNewGameObject().AsSingle();
        }
    }
}
