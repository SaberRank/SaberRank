#pragma once

#include "Features/Live/Compete/UI/FlowCoordinators/CompeteFlowCoordinator.hpp"
#include "Features/MainMenu/MainFlow/SnoreSaberFlowCoordinator.hpp"
#include "Features/MainMenu/Settings/SnoreSaberSettingsFlowCoordinator.hpp"

#include <GlobalNamespace/MainFlowCoordinator.hpp>
#include <HMUI/FlowCoordinator.hpp>
#include <System/zzzz__Object_def.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::MainMenu, SnoreSaberMenuNavigator, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MainFlowCoordinator*, _mainFlowCoordinator);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::UI::FlowCoordinators::SnoreSaberFlowCoordinator*, _scoreSaberFlowCoordinator);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::UI::FlowCoordinators::SnoreSaberSettingsFlowCoordinator*, _settingsFlowCoordinator);
    DECLARE_INSTANCE_FIELD_PRIVATE(::SnoreSaber::Features::Live::Compete::UI::FlowCoordinators::CompeteFlowCoordinator*, _competeFlowCoordinator);
    DECLARE_INSTANCE_FIELD_PRIVATE(HMUI::FlowCoordinator*, _activeTournamentFlowCoordinator);
    DECLARE_INSTANCE_FIELD_PRIVATE(HMUI::FlowCoordinator*, _tournamentPresentingFlowCoordinator);
    DECLARE_CTOR(ctor,
                 GlobalNamespace::MainFlowCoordinator* mainFlowCoordinator,
                 SnoreSaber::UI::FlowCoordinators::SnoreSaberFlowCoordinator* scoreSaberFlowCoordinator,
                 SnoreSaber::UI::FlowCoordinators::SnoreSaberSettingsFlowCoordinator* settingsFlowCoordinator,
                 ::SnoreSaber::Features::Live::Compete::UI::FlowCoordinators::CompeteFlowCoordinator* competeFlowCoordinator);

  public:
    void ShowMain();
    void ShowSettings();
    void ShowCompete();

  private:
    HMUI::FlowCoordinator* ActiveFlowCoordinator();
    void Present(HMUI::FlowCoordinator* activeFlowCoordinator, HMUI::FlowCoordinator* flowCoordinator);
    void PresentTournamentFlow(HMUI::FlowCoordinator* flowCoordinator);
    void TournamentFlowDidFinish();
};
