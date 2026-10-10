#pragma once

#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/Domain/CompeteTournament.hpp"
#include "Utils/Event.hpp"

#include <HMUI/TableView.hpp>
#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/UI/Button.hpp>
#include <bsml/shared/BSML/Components/CustomCellListTableData.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Rooms, CompeteRoomListViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::CustomCellListTableData>, roomList);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, refreshingStateObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, emptyStateObject);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, titleText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, subtitleText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::UI::Button>, refreshButton);
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(ListW<System::Object*>, rooms, ListW<System::Object*>::New());

    DECLARE_INSTANCE_METHOD(void, RefreshClicked);
    DECLARE_INSTANCE_METHOD(void, SelectRoom, HMUI::TableView* tableView, int idx);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    Utils::Event<> RefreshRequested;
    Utils::Event<const Domain::CompeteRoom&> RoomSelected;

    void SetTournament(const Domain::CompeteTournament& tournament);
    void SetRooms(const std::vector<Domain::CompeteRoom>& items);
    void SetRefreshing(bool value);
    void SetStatus(const std::string& value);

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    std::vector<Domain::CompeteRoom> _roomItems;
    std::string _title = "Rooms";
    std::string _subtitle;
    bool _hasRooms = false;
    bool _refreshing = false;
    std::unordered_set<void*> _fixedCells;

    void ApplyTexts();
    void ApplyVisibility();
    void ReloadList();
    void FixCellInteractivity();
};
