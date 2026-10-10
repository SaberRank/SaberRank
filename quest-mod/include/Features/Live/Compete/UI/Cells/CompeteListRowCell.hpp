#pragma once

#include <HMUI/ImageView.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <bsml/shared/BSML/Components/CustomCellTableCell.hpp>

#include <string>

namespace SnoreSaber::Features::Live::Compete::UI::Cells
{
    // pc binds row values through a CompeteListRowCell base class; quest custom cells
    // fill tagged components when bsml calls Setup/Reused on the data object instead.
    // the pc refresh-visuals separator recolor is handled by the selected/hovered/
    // un-selected-un-hovered tag toggling baked into the cell templates.
    TMPro::TextMeshProUGUI* FindTaggedText(BSML::CustomCellTableCell* cell, const std::string& tag);
    HMUI::ImageView* FindTaggedImage(BSML::CustomCellTableCell* cell, const std::string& tag);
    void ApplyRowTexts(BSML::CustomCellTableCell* cell, const std::string& title, const std::string& detail, const std::string& status);
}
