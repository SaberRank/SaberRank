#include "Features/Live/Compete/UI/Cells/CompeteListRowCell.hpp"

#include <UnityEngine/GameObject.hpp>

namespace SnoreSaber::Features::Live::Compete::UI::Cells
{
    namespace
    {
        UnityEngine::GameObject* FindTaggedObject(BSML::CustomCellTableCell* cell, const std::string& tag)
        {
            if (!cell || !cell->parserParams)
            {
                return nullptr;
            }

            const auto& objects = cell->parserParams->GetObjectsWithTag(tag);
            if (objects.empty())
            {
                return nullptr;
            }

            return objects.front();
        }
    }

    TMPro::TextMeshProUGUI* FindTaggedText(BSML::CustomCellTableCell* cell, const std::string& tag)
    {
        auto object = FindTaggedObject(cell, tag);
        if (!object)
        {
            return nullptr;
        }

        return object->GetComponent<TMPro::TextMeshProUGUI*>();
    }

    HMUI::ImageView* FindTaggedImage(BSML::CustomCellTableCell* cell, const std::string& tag)
    {
        auto object = FindTaggedObject(cell, tag);
        if (!object)
        {
            return nullptr;
        }

        return object->GetComponent<HMUI::ImageView*>();
    }

    void ApplyRowTexts(BSML::CustomCellTableCell* cell, const std::string& title, const std::string& detail, const std::string& status)
    {
        auto titleText = FindTaggedText(cell, "row-title");
        if (titleText)
        {
            titleText->text = title;
        }

        auto detailText = FindTaggedText(cell, "row-detail");
        if (detailText)
        {
            detailText->text = detail;
        }

        auto statusText = FindTaggedText(cell, "row-status");
        if (statusText)
        {
            statusText->text = status;
        }

        // sync the selected/hovered tag objects; fresh cells otherwise show every
        // separator layer until the first selection or highlight change
        if (cell)
        {
            cell->RefreshVisuals();
        }
    }
}
