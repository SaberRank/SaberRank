#include "Features/Live/UI/ViewControllers/TournamentBrowserViewController.hpp"

#include "Features/Live/Compete/UI/Cells/CompeteTournamentCell.hpp"
#include "assets.hpp"
#include "logging.hpp"

#include <HMUI/SelectableCell.hpp>
#include <HMUI/TableCell.hpp>
#include <HMUI/TableViewSelectionType.hpp>
#include <System/Action_1.hpp>
#include <System/Action_3.hpp>
#include <bsml/shared/BSML.hpp>
#include <custom-types/shared/delegate.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live::UI::ViewControllers, TournamentBrowserViewController);

namespace SnoreSaber::Features::Live::UI::ViewControllers
{
    void TournamentBrowserViewController::ctor()
    {
        INVOKE_CTOR();
    }

    void TournamentBrowserViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::TournamentBrowserViewController_bsml, transform, this);
        }

        ApplyVisibility();
        ReloadList();
    }

    void TournamentBrowserViewController::SetTournaments(const std::vector<Compete::Domain::CompeteTournament>& items)
    {
        _tournamentItems = items;
        tournaments->Clear();
        for (auto& tournament : _tournamentItems)
        {
            tournaments->Add(Compete::UI::Cells::CompeteTournamentCell::Create(tournament));
        }

        _hasTournaments = !_tournamentItems.empty();
        ApplyVisibility();
        ReloadList();
    }

    void TournamentBrowserViewController::RefreshClicked()
    {
        RefreshRequested.Invoke();
    }

    void TournamentBrowserViewController::SelectTournament(HMUI::TableView* tableView, int idx)
    {
        INFO("Tournament cell selected: {}", idx);
        if (idx < 0 || idx >= static_cast<int>(_tournamentItems.size()))
        {
            return;
        }

        TournamentSelected.Invoke(_tournamentItems[idx]);
    }

    void TournamentBrowserViewController::ApplyVisibility()
    {
        if (tournamentList)
        {
            tournamentList->gameObject->SetActive(_hasTournaments);
        }

        if (emptyStateObject)
        {
            emptyStateObject->SetActive(!_hasTournaments);
        }
    }

    void TournamentBrowserViewController::ReloadList()
    {
        if (!tournamentList || !tournamentList->tableView)
        {
            return;
        }

        auto tableView = tournamentList->tableView;
        // temp diagnostics: quest bsml never sets a selection type on custom lists, log the
        // ctor default then force single selection so cell clicks can dispatch
        INFO("Tournament list reload: {} items, tableView selectionType was {}", _tournamentItems.size(), static_cast<int>(tableView->_selectionType.value__));
        tableView->set_selectionType(HMUI::TableViewSelectionType::Single);
        tableView->ReloadData();
        tableView->ClearSelection();
        InstrumentCells();
    }

    void TournamentBrowserViewController::InstrumentCells()
    {
        // temp diagnostics: figure out how far a pointer click gets into the custom cells
        auto visibleCells = ListW<HMUI::TableCell*>(tournamentList->tableView->_visibleCells);
        if (!visibleCells)
        {
            INFO("Tournament list has no visible cell list");
            return;
        }

        INFO("Tournament list visible cells after reload: {}", visibleCells.size());
        for (auto cell : visibleCells)
        {
            if (!cell)
            {
                continue;
            }

            // quest bsml custom cells end up non-interactable by click time; force the flag
            // each reload and log whether it sticks
            bool wasInteractable = cell->get_interactable();
            cell->set_interactable(true);
            INFO("Tournament cell idx {}: interactable {} -> {}", cell->get_idx(), wasInteractable, cell->get_interactable());

            if (_instrumentedCells.contains(cell))
            {
                continue;
            }

            _instrumentedCells.insert(cell);

            cell->add_selectionDidChangeEvent(custom_types::MakeDelegate<System::Action_3<UnityW<HMUI::SelectableCell>, HMUI::SelectableCell::TransitionType, System::Object*>*>(
                std::function<void(UnityW<HMUI::SelectableCell>, HMUI::SelectableCell::TransitionType, System::Object*)>([](UnityW<HMUI::SelectableCell> changed, HMUI::SelectableCell::TransitionType, System::Object*)
                    { INFO("Tournament cell selection changed: {}", changed ? changed->get_selected() : false); })));

            // fallback: if the click still gets rejected as non-interactable, dispatch the
            // selection manually so the bsml select-cell event fires either way
            HMUI::TableCell* rawCell = cell;
            cell->add_nonInteractableCellWasPressedEvent(custom_types::MakeDelegate<System::Action_1<UnityW<HMUI::SelectableCell>>*>(
                std::function<void(UnityW<HMUI::SelectableCell>)>([this, rawCell](UnityW<HMUI::SelectableCell>)
                    {
                        int idx = rawCell->get_idx();
                        INFO("Tournament cell idx {} pressed while non-interactable (interactable={}); dispatching selection manually", idx, rawCell->get_interactable());
                        if (tournamentList && tournamentList->tableView)
                        {
                            tournamentList->tableView->SelectCellWithIdx(idx, true);
                        }
                    })));
        }
    }
}
