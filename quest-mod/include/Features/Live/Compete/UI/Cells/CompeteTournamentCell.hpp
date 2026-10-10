#pragma once

#include "Features/Live/Compete/Domain/CompeteTournament.hpp"

#include <bsml/shared/BSML/Components/CustomCellTableCell.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::Cells, CompeteTournamentCell, System::Object) {
    DECLARE_INSTANCE_METHOD(void, Setup, BSML::CustomCellTableCell* cell);
    DECLARE_INSTANCE_METHOD(void, Reused, BSML::CustomCellTableCell* cell);
    DECLARE_INSTANCE_METHOD(void, Select, BSML::CustomCellTableCell* cell);
    DECLARE_CTOR(ctor);

  public:
    static CompeteTournamentCell* Create(Domain::CompeteTournament tournament);

    Domain::CompeteTournament tournament;
};
