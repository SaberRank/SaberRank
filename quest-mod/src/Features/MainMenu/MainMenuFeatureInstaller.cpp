#include "Features/MainMenu/MainMenuFeatureInstaller.hpp"

#include "Features/MainMenu/MainFlow/FAQ/FAQViewController.hpp"
#include "Features/MainMenu/MainFlow/GlobalLeaderboard/GlobalViewController.hpp"
#include "Features/MainMenu/MainFlow/SnoreSaberFlowCoordinator.hpp"
#include "Features/MainMenu/MainFlow/Teams/UI/TeamViewController.hpp"
#include "Features/MainMenu/SnoreSaberMenuNavigator.hpp"
#include "Features/MainMenu/Settings/SnoreSaberSettingsFlowCoordinator.hpp"
#include "Features/MainMenu/Settings/ViewControllers/MainSettingsViewController.hpp"

#include <Zenject/ConcreteBinderGeneric_1.hpp>
#include <Zenject/ConcreteIdBinderGeneric_1.hpp>
#include <Zenject/DiContainer.hpp>
#include <Zenject/FromBinderNonGeneric.hpp>
#include <lapiz/shared/utilities/ZenjectExtensions.hpp>

DEFINE_TYPE(SnoreSaber::Features::MainMenu, MainMenuFeatureInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace SnoreSaber::Features::MainMenu
{
    void MainMenuFeatureInstaller::InstallBindings()
    {
        auto container = Container;
        FromNewComponentAsViewController(container->Bind<SnoreSaber::UI::ViewControllers::TeamViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<SnoreSaber::UI::ViewControllers::FAQViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<SnoreSaber::UI::ViewControllers::GlobalViewController*>())->AsSingle();
        FromNewComponentAsViewController(container->Bind<SnoreSaber::UI::ViewControllers::MainSettingsViewController*>())->AsSingle();

        FromNewComponentOnNewGameObject(container->Bind<SnoreSaber::UI::FlowCoordinators::SnoreSaberFlowCoordinator*>())->AsSingle();
        FromNewComponentOnNewGameObject(container->Bind<SnoreSaber::UI::FlowCoordinators::SnoreSaberSettingsFlowCoordinator*>())->AsSingle();
        container->Bind<SnoreSaberMenuNavigator*>()->AsSingle();
    }
}
