#pragma once

#include "Core/Configuration/SettingsService.hpp"
#include "Features/Leaderboards/Services/LeaderboardScreenSession.hpp"
#include "Features/Players/Services/LocalPlayerPanelSession.hpp"
#include <HMUI/FlowCoordinator.hpp>
#include <HMUI/ViewController.hpp>
#include "Features/MainMenu/Settings/ViewControllers/MainSettingsViewController.hpp"
#include <UnityEngine/GameObject.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::FlowCoordinators, SnoreSaberSettingsFlowCoordinator, HMUI::FlowCoordinator) {
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::FlowCoordinator::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, BackButtonWasPressed, &HMUI::FlowCoordinator::BackButtonWasPressed, HMUI::ViewController* topViewController);

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityW<SnoreSaber::UI::ViewControllers::MainSettingsViewController>, mainSettingsViewController, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityW<HMUI::FlowCoordinator>, presentingFlowCoordinator, nullptr);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Core::Configuration::SettingsService*, _settingsService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenSession*, _leaderboardSession);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::LocalPlayerPanelSession*, _localPlayerPanelSession);

    DECLARE_CTOR(ctor,
                 SnoreSaber::UI::ViewControllers::MainSettingsViewController* mainSettingsViewController,
                 SnoreSaber::Core::Configuration::SettingsService* settingsService,
                 SnoreSaber::Features::Leaderboards::Services::LeaderboardScreenSession* leaderboardSession,
                 SnoreSaber::Features::Players::Services::LocalPlayerPanelSession* localPlayerPanelSession);

  public:
    void SetPresentingFlowCoordinator(HMUI::FlowCoordinator* flowCoordinator);
};
