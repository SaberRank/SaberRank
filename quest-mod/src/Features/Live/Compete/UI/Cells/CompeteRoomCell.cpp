#include "Features/Live/Compete/UI/Cells/CompeteRoomCell.hpp"

#include "Features/Live/Compete/UI/Cells/CompeteListRowCell.hpp"

#include <fmt/core.h>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::Cells, CompeteRoomCell);

namespace SnoreSaber::Features::Live::Compete::UI::Cells
{
    namespace
    {
        std::string PlayerCountText(const SnoreSaber::Features::Live::Compete::Domain::CompeteRoom& room)
        {
            if (room.playerCount == 1)
            {
                return "1 player";
            }

            return fmt::format("{} players", room.playerCount);
        }
    }

    void CompeteRoomCell::ctor()
    {
        INVOKE_CTOR();
    }

    CompeteRoomCell* CompeteRoomCell::Create(Domain::CompeteRoom room)
    {
        auto cell = CompeteRoomCell::New_ctor();
        cell->room = std::move(room);
        return cell;
    }

    void CompeteRoomCell::Setup(BSML::CustomCellTableCell* cell)
    {
        ApplyRowTexts(cell, room.DisplayName(), fmt::format("{} - {}", room.round, PlayerCountText(room)), room.state);
    }

    void CompeteRoomCell::Reused(BSML::CustomCellTableCell* cell)
    {
        Setup(cell);
    }

    void CompeteRoomCell::Select(BSML::CustomCellTableCell* cell)
    {
    }
}
