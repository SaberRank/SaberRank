#include "Features/Live/LiveFeatureInstaller.hpp"

#include "Features/Live/Compete/Services/CompeteDirectoryService.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayLauncher.hpp"
#include "Features/Live/Compete/Services/CompeteSongService.hpp"
#include "Features/Live/Compete/UI/FlowCoordinators/CompeteFlowCoordinator.hpp"
#include "Features/Live/Compete/UI/ViewControllers/CodeEntry/CompeteCodeEntryViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Entry/CompeteModeSelectionViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Room/Center/CompeteRoomViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Room/Left/CompetePlayerListViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Rooms/CompeteRoomListViewController.hpp"
#include "Features/Live/Compete/UI/ViewControllers/Shared/CompeteLoadingViewController.hpp"
#include "Features/Live/Ludus/Services/LiveChatLinkService.hpp"
#include "Features/Live/Ludus/Services/LiveChatSongNavigator.hpp"
#include "Features/Live/Ludus/Services/LudusSessionService.hpp"
#include "Features/Live/Ludus/UI/LiveChatFloatingViewController.hpp"
#include "Features/Live/Ludus/UI/LiveChatOverlayController.hpp"
#include "Features/Live/UI/ViewControllers/TournamentBrowserViewController.hpp"

#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>
#include <lapiz/shared/utilities/ZenjectExtensions.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live, LiveFeatureInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace SnoreSaber::Features::Live
{
    void LiveFeatureInstaller::InstallBindings()
    {
        auto container = Container;
        FromNewComponentAsViewController(container->Bind<Compete::UI::ViewControllers::Entry::CompeteModeSelectionViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<SnoreSaber::Features::Live::UI::ViewControllers::TournamentBrowserViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<Compete::UI::ViewControllers::Rooms::CompeteRoomListViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<Compete::UI::ViewControllers::Room::Center::CompeteRoomViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<Compete::UI::ViewControllers::Room::Left::CompetePlayerListViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<Ludus::UI::LiveChatFloatingViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<Compete::UI::ViewControllers::CodeEntry::CompeteCodeEntryViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<Compete::UI::ViewControllers::Shared::CompeteLoadingViewController*>())->AsSingle();
        container->Bind<Compete::Services::CompeteDirectoryService*>()->AsSingle();
        container->Bind<Compete::Services::CompeteSongService*>()->AsSingle();
        container->Bind<Ludus::Services::LiveChatSongNavigator*>()->AsSingle();
        container->Bind<Ludus::Services::LiveChatLinkService*>()->AsSingle();
        container->Bind<Compete::Services::CompeteGameplayLauncher*>()->AsSingle();
        container->BindInterfacesAndSelfTo<Ludus::Services::LudusSessionService*>()->AsSingle()->NonLazy();
        container->BindInterfacesTo<Ludus::UI::LiveChatOverlayController*>()->AsSingle()->NonLazy();
        FromNewComponentOnNewGameObject(container->Bind<Compete::UI::FlowCoordinators::CompeteFlowCoordinator*>())->AsSingle();
    }
}
