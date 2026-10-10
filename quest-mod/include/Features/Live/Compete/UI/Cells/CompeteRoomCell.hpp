#pragma once

#include "Features/Live/Compete/Domain/CompeteRoom.hpp"

#include <bsml/shared/BSML/Components/CustomCellTableCell.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::Cells, CompeteRoomCell, System::Object) {
    DECLARE_INSTANCE_METHOD(void, Setup, BSML::CustomCellTableCell* cell);
    DECLARE_INSTANCE_METHOD(void, Reused, BSML::CustomCellTableCell* cell);
    DECLARE_INSTANCE_METHOD(void, Select, BSML::CustomCellTableCell* cell);
    DECLARE_CTOR(ctor);

  public:
    static CompeteRoomCell* Create(Domain::CompeteRoom room);

    Domain::CompeteRoom room;
};
