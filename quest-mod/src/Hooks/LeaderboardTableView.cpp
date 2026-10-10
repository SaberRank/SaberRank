#include "hooks.hpp"

#include "Features/Leaderboards/UI/Components/CellClicker.hpp"
#include <GlobalNamespace/LeaderboardTableCell.hpp>
#include <GlobalNamespace/LeaderboardTableView.hpp>
#include <HMUI/TableCell.hpp>
#include <HMUI/TableView.hpp>
#include <TMPro/TextMeshPro.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/UI/CanvasUpdate.hpp>
#include <cstddef>

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace SnoreSaber::UI::Other;
using namespace SnoreSaber::CustomTypes::Components;

std::optional<Vector2> normalAnchor;

MAKE_AUTO_HOOK_MATCH(LeaderboardTableView_CellForIdx,
                     &::GlobalNamespace::LeaderboardTableView::CellForIdx, UnityW<HMUI::TableCell>, GlobalNamespace::LeaderboardTableView* self,
                     HMUI::TableView* tableView, int row)
{
    HMUI::TableCell* tableCell = LeaderboardTableView_CellForIdx(self, tableView, row);
    LeaderboardTableCell* cell = il2cpp_utils::try_cast<LeaderboardTableCell>(tableCell).value_or(nullptr);
    if (!cell || !cell->_playerNameText || !cell->_playerNameText->rectTransform) {
        return tableCell;
    }

    if (!normalAnchor) {
        normalAnchor = cell->_playerNameText->rectTransform->anchoredPosition;
    }

    cell->_playerNameText->richText = true;
    cell->_playerNameText->rectTransform->anchoredPosition = Vector2(normalAnchor->x + 2.5f, 0.0f);
    cell->_playerNameText->Rebuild(UnityEngine::UI::CanvasUpdate::PreRender);
    cell->showSeparator = true;
    if (cell->_separatorImage) {
        cell->_separatorImage->gameObject->SetActive(true);
        cell->_separatorImage->enabled = true;
    }

    if (row >= 0 && static_cast<std::size_t>(row) < SnoreSaberLeaderboardView::_cellClickingImages.size()) {
        auto clickImage = SnoreSaberLeaderboardView::_cellClickingImages[static_cast<std::size_t>(row)];
        if (!clickImage || !clickImage.ptr() || !clickImage->gameObject || !SnoreSaberLeaderboardView::leaderboardScoreInfoButtonHandler) {
            return cell;
        }

        CellClicker* cellClicker = clickImage->gameObject->GetComponent<CellClicker*>();
        if (!cellClicker) {
            cellClicker = clickImage->gameObject->AddComponent<CellClicker*>();
        }
        cellClicker->Configure(row, cell->_separatorImage.cast<HMUI::ImageView>(),
            std::bind(&LeaderboardScoreInfoButtonHandler::ShowScoreInfoModal, SnoreSaberLeaderboardView::leaderboardScoreInfoButtonHandler, std::placeholders::_1));
    }

    return cell;
}
