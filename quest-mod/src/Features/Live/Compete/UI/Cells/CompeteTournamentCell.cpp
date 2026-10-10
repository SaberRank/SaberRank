#include "Features/Live/Compete/UI/Cells/CompeteTournamentCell.hpp"

#include "Features/Live/Compete/UI/Cells/CompeteListRowCell.hpp"

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::Cells, CompeteTournamentCell);

namespace SnoreSaber::Features::Live::Compete::UI::Cells
{
    void CompeteTournamentCell::ctor()
    {
        INVOKE_CTOR();
    }

    CompeteTournamentCell* CompeteTournamentCell::Create(Domain::CompeteTournament tournament)
    {
        auto cell = CompeteTournamentCell::New_ctor();
        cell->tournament = std::move(tournament);
        return cell;
    }

    void CompeteTournamentCell::Setup(BSML::CustomCellTableCell* cell)
    {
        ApplyRowTexts(cell, tournament.name, tournament.roomSummary, "");
    }

    void CompeteTournamentCell::Reused(BSML::CustomCellTableCell* cell)
    {
        Setup(cell);
    }

    void CompeteTournamentCell::Select(BSML::CustomCellTableCell* cell)
    {
    }
}
