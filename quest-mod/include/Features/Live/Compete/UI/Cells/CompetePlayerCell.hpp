#pragma once

#include "Core/Presentation/SnoreSaberUIMaterials.hpp"
#include "Features/Live/Compete/Domain/CompetePlayer.hpp"

#include <bsml/shared/BSML/Components/CustomCellTableCell.hpp>
#include <custom-types/shared/macros.hpp>

#include <functional>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::Cells, CompetePlayerCell, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Presentation::SnoreSaberUIMaterials*, _materials);

    DECLARE_INSTANCE_METHOD(void, Setup, BSML::CustomCellTableCell* cell);
    DECLARE_INSTANCE_METHOD(void, Reused, BSML::CustomCellTableCell* cell);
    DECLARE_INSTANCE_METHOD(void, Select, BSML::CustomCellTableCell* cell);
    DECLARE_CTOR(ctor);

  public:
    static CompetePlayerCell* Create(Domain::CompetePlayer player, Core::Presentation::SnoreSaberUIMaterials* materials,
                                     std::function<void(const std::string&, const std::string&)> profileClicked);

    Domain::CompetePlayer player;

  private:
    std::function<void(const std::string&, const std::string&)> _profileClicked;

    void Apply(BSML::CustomCellTableCell* cell, bool firstSetup);
    void ProfileClicked();
};
