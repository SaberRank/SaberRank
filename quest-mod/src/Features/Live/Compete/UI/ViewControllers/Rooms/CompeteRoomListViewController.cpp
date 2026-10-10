#include "Features/Live/Compete/UI/ViewControllers/Rooms/CompeteRoomListViewController.hpp"

#include "Features/Live/Compete/UI/Cells/CompeteRoomCell.hpp"
#include "assets.hpp"

#include <HMUI/SelectableCell.hpp>
#include <HMUI/TableCell.hpp>
#include <HMUI/TableViewSelectionType.hpp>
#include <System/Action_1.hpp>
#include <bsml/shared/BSML.hpp>
#include <custom-types/shared/delegate.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Rooms, CompeteRoomListViewController);

namespace SnoreSaber::Features::Live::Compete::UI::ViewControllers::Rooms
{
    namespace
    {
        constexpr const char* DefaultSubtitle = "Rooms you have permission to join";
    }

    void CompeteRoomListViewController::ctor()
    {
        INVOKE_CTOR();
        _subtitle = DefaultSubtitle;
    }

    void CompeteRoomListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::CompeteRoomListViewController_bsml, transform, this);
        }

        ApplyTexts();
        ApplyVisibility();
        ReloadList();
    }

    void CompeteRoomListViewController::SetTournament(const Domain::CompeteTournament& tournament)
    {
        _title = tournament.name;
        _subtitle = DefaultSubtitle;
        ApplyTexts();
    }

    void CompeteRoomListViewController::SetRooms(const std::vector<Domain::CompeteRoom>& items)
    {
        _roomItems = items;
        rooms->Clear();
        for (auto& room : _roomItems)
        {
            rooms->Add(Cells::CompeteRoomCell::Create(room));
        }

        _hasRooms = !_roomItems.empty();
        ApplyVisibility();
        ReloadList();
    }

    void CompeteRoomListViewController::SetRefreshing(bool value)
    {
        _refreshing = value;
        ApplyVisibility();
        ReloadList();
    }

    void CompeteRoomListViewController::SetStatus(const std::string& value)
    {
        if (value.empty())
        {
            _subtitle = DefaultSubtitle;
        }
        else
        {
            _subtitle = value;
        }

        ApplyTexts();
    }

    void CompeteRoomListViewController::RefreshClicked()
    {
        if (_refreshing)
        {
            return;
        }

        RefreshRequested.Invoke();
    }

    void CompeteRoomListViewController::SelectRoom(HMUI::TableView* tableView, int idx)
    {
        if (_refreshing)
        {
            return;
        }

        if (idx < 0 || idx >= static_cast<int>(_roomItems.size()))
        {
            return;
        }

        RoomSelected.Invoke(_roomItems[idx]);
    }

    void CompeteRoomListViewController::ApplyTexts()
    {
        if (titleText)
        {
            titleText->text = _title;
        }

        if (subtitleText)
        {
            subtitleText->text = _subtitle;
        }
    }

    void CompeteRoomListViewController::ApplyVisibility()
    {
        if (roomList)
        {
            roomList->gameObject->SetActive(_hasRooms && !_refreshing);
        }

        if (refreshingStateObject)
        {
            refreshingStateObject->SetActive(_refreshing);
        }

        if (emptyStateObject)
        {
            emptyStateObject->SetActive(!_hasRooms && !_refreshing);
        }

        if (refreshButton)
        {
            refreshButton->interactable = !_refreshing;
        }
    }

    void CompeteRoomListViewController::ReloadList()
    {
        if (!roomList || !roomList->tableView)
        {
            return;
        }

        // quest bsml never sets a selection type on custom lists; force single selection
        roomList->tableView->set_selectionType(HMUI::TableViewSelectionType::Single);
        roomList->tableView->ReloadData();
        roomList->tableView->ClearSelection();
        FixCellInteractivity();
    }

    void CompeteRoomListViewController::FixCellInteractivity()
    {
        // quest bsml custom cells end up non-interactable by click time; force the flag and
        // dispatch selection manually if a press still gets rejected
        auto visibleCells = ListW<HMUI::TableCell*>(roomList->tableView->_visibleCells);
        if (!visibleCells)
        {
            return;
        }

        for (auto cell : visibleCells)
        {
            if (!cell)
            {
                continue;
            }

            cell->set_interactable(true);
            if (_fixedCells.contains(cell))
            {
                continue;
            }

            _fixedCells.insert(cell);
            HMUI::TableCell* rawCell = cell;
            cell->add_nonInteractableCellWasPressedEvent(custom_types::MakeDelegate<System::Action_1<UnityW<HMUI::SelectableCell>>*>(
                std::function<void(UnityW<HMUI::SelectableCell>)>([this, rawCell](UnityW<HMUI::SelectableCell>)
                    {
                        if (roomList && roomList->tableView)
                        {
                            roomList->tableView->SelectCellWithIdx(rawCell->get_idx(), true);
                        }
                    })));
        }
    }
}
