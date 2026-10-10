#pragma once

#include "Features/Live/Compete/Domain/CompeteTournament.hpp"
#include "Utils/Event.hpp"

#include <HMUI/TableView.hpp>
#include <HMUI/ViewController.hpp>
#include <UnityEngine/GameObject.hpp>
#include <bsml/shared/BSML/Components/CustomCellListTableData.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <unordered_set>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::UI::ViewControllers, TournamentBrowserViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<BSML::CustomCellListTableData>, tournamentList);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<UnityEngine::GameObject>, emptyStateObject);
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(ListW<System::Object*>, tournaments, ListW<System::Object*>::New());

    DECLARE_INSTANCE_METHOD(void, RefreshClicked);
    DECLARE_INSTANCE_METHOD(void, SelectTournament, HMUI::TableView* tableView, int idx);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    Utils::Event<> RefreshRequested;
    Utils::Event<const Compete::Domain::CompeteTournament&> TournamentSelected;

    void SetTournaments(const std::vector<Compete::Domain::CompeteTournament>& items);

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    std::vector<Compete::Domain::CompeteTournament> _tournamentItems;
    bool _hasTournaments = false;
    std::unordered_set<void*> _instrumentedCells;

    void ApplyVisibility();
    void ReloadList();
    void InstrumentCells();
};
