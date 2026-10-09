using SnoreSaber.Features.Live.Ludus.Services;
using SnoreSaber.Features.Live.Compete.Services;
using SnoreSaber.Features.Live.Compete.UI.FlowCoordinators;
using SnoreSaber.Features.Live.Compete.UI.ViewControllers.CodeEntry;
using SnoreSaber.Features.Live.Compete.UI.ViewControllers.Entry;
using SnoreSaber.Features.Live.Compete.UI.ViewControllers.Room.Center;
using SnoreSaber.Features.Live.Compete.UI.ViewControllers.Room.Left;
using SnoreSaber.Features.Live.Compete.UI.ViewControllers.Rooms;
using SnoreSaber.Features.Live.Compete.UI.ViewControllers.Shared;
using SnoreSaber.Features.Live.Ludus.UI;
using SnoreSaber.Features.Live.UI.ViewControllers;
using Zenject;

namespace SnoreSaber.Features.Live {
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
