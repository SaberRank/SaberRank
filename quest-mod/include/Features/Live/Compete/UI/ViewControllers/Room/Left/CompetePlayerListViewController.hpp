#pragma once

#include "Core/Presentation/SnoreSaberUIMaterials.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Players/Profile/PlayerProfileModal.hpp"

#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/GameObject.hpp>
#include <bsml/shared/BSML/Components/CustomCellListTableData.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Room::Left, CompetePlayerListViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::CustomCellListTableData>, playerList);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::CustomCellListTableData>, teamOnePlayerList);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::CustomCellListTableData>, teamTwoPlayerList);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, teamPlayersObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, emptyStateObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, teamOneNameText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, teamTwoNameText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<::SnoreSaber::UI::Other::PlayerProfileModal>, profileModal);
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Presentation::SnoreSaberUIMaterials*, _materials);
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(ListW<System::Object*>, players, ListW<System::Object*>::New());
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(ListW<System::Object*>, teamOnePlayers, ListW<System::Object*>::New());
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(ListW<System::Object*>, teamTwoPlayers, ListW<System::Object*>::New());

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    void SetRoom(const Domain::CompeteRoom& room);

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    Domain::CompeteTeam _teamOne;
    Domain::CompeteTeam _teamTwo;
    bool _hasPlayers = false;
    bool _teamMode = false;

    void ShowProfile(const std::string& playerId, const std::string& name);
    void ApplyTeamNames();
    void ApplyVisibility();
    void ReloadPlayers();
    void MoveTeamOneScrollbarToLeft();
};
