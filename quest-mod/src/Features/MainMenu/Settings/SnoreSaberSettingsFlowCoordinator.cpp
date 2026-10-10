#include "Features/MainMenu/Settings/SnoreSaberSettingsFlowCoordinator.hpp"

#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"

#include <bsml/shared/Helpers/creation.hpp>
#include <HMUI/ViewController.hpp>

DEFINE_TYPE(SnoreSaber::UI::FlowCoordinators, SnoreSaberSettingsFlowCoordinator);

using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace HMUI;
using namespace BSML::Helpers;
using namespace SnoreSaber::UI::Other;

namespace SnoreSaber::UI::FlowCoordinators
{
    void SnoreSaberSettingsFlowCoordinator::ctor(SnoreSaber::UI::ViewControllers::MainSettingsViewController* mainSettingsViewController,
                                                 SnoreSaber::Core::Configuration::SettingsService* settingsService,
                                                 SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenSession* leaderboardSession,
                                                 SnoreSaber::Features::Players::Services::LocalPlayerPanelSession* localPlayerPanelSession)
    {
        INVOKE_CTOR();
        this->mainSettingsViewController = mainSettingsViewController;
        _settingsService = settingsService;
        _leaderboardSession = leaderboardSession;
        _localPlayerPanelSession = localPlayerPanelSession;
    }

    void SnoreSaberSettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            SetTitle("SnoreSaber Settings", ViewController::AnimationType::In);
            showBackButton = true;
            if (!mainSettingsViewController)
            {
                mainSettingsViewController = CreateViewController<SnoreSaber::UI::ViewControllers::MainSettingsViewController*>();
            }

            ProvideInitialViewControllers(mainSettingsViewController, nullptr, nullptr, nullptr, nullptr);
        }
    }

    void SnoreSaberSettingsFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController)
    {
        SetLeftScreenViewController(nullptr, ViewController::AnimationType::None);
        SetRightScreenViewController(nullptr, ViewController::AnimationType::None);
        _settingsService->Save();
        HMUI::FlowCoordinator* flowCoordinator = nullptr;
        if (presentingFlowCoordinator)
        {
            flowCoordinator = presentingFlowCoordinator.unsafePtr();
        }
        else if (this->_parentFlowCoordinator)
        {
            flowCoordinator = this->_parentFlowCoordinator.unsafePtr();
        }
        if (flowCoordinator)
        {
            flowCoordinator->DismissFlowCoordinator(this, ViewController::AnimationDirection::Horizontal, nullptr, false);
        }
        if (_leaderboardSession)
        {
            _leaderboardSession->RefreshFromFirstPage();
        }
        SnoreSaberLeaderboardView::RefreshLeaderboard();
        if (_localPlayerPanelSession)
        {
            _localPlayerPanelSession->ApplyCurrentSettings();
        }
    }

    void SnoreSaberSettingsFlowCoordinator::SetPresentingFlowCoordinator(HMUI::FlowCoordinator* flowCoordinator)
    {
        presentingFlowCoordinator = flowCoordinator;
    }
}
