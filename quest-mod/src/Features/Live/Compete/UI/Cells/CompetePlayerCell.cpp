#include "Features/Live/Compete/UI/Cells/CompetePlayerCell.hpp"

#include "Features/Live/Compete/UI/Cells/CompeteListRowCell.hpp"
#include "Sprites.hpp"

#include <bsml/shared/BSML-Lite.hpp>
#include <bsml/shared/BSML/Components/ClickableImage.hpp>
#include <bsml/shared/BSML/Components/ClickableText.hpp>
#include <bsml/shared/Helpers/utilities.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::Cells, CompetePlayerCell);

namespace SnoreSaber::Features::Live::Compete::UI::Cells
{
    void CompetePlayerCell::ctor()
    {
        INVOKE_CTOR();
    }

    CompetePlayerCell* CompetePlayerCell::Create(Domain::CompetePlayer player, Core::Presentation::SnoreSaberUIMaterials* materials,
                                                 std::function<void(const std::string&, const std::string&)> profileClicked)
    {
        auto cell = CompetePlayerCell::New_ctor();
        cell->player = std::move(player);
        cell->_materials = materials;
        cell->_profileClicked = std::move(profileClicked);
        return cell;
    }

    void CompetePlayerCell::Setup(BSML::CustomCellTableCell* cell)
    {
        Apply(cell, true);
    }

    void CompetePlayerCell::Reused(BSML::CustomCellTableCell* cell)
    {
        Apply(cell, false);
    }

    void CompetePlayerCell::Select(BSML::CustomCellTableCell* cell)
    {
    }

    void CompetePlayerCell::Apply(BSML::CustomCellTableCell* cell, bool firstSetup)
    {
        std::string title = player.isLocalPlayer ? player.DisplayName() + " (you)" : player.DisplayName();
        ApplyRowTexts(cell, title, player.rank, player.status);

        auto image = FindTaggedImage(cell, "profile-image");
        if (image)
        {
            if (_materials)
            {
                image->material = _materials->RoundedImageMaterial();
            }

            // pc binds avatar-url and falls back to the bundled user.png; quest loads
            // through bsml utilities with the oculus default sprite as the fallback
            if (player.avatarUrl.empty())
            {
                image->sprite = BSML::Lite::Base64ToSprite(oculus_base64);
            }
            else
            {
                BSML::Utilities::SetImage(image, player.avatarUrl);
            }
        }

        if (firstSetup)
        {
            // resolve the data object at click time; reused cells swap dataObject
            auto clickHandler = [cell]() {
                auto data = il2cpp_utils::try_cast<CompetePlayerCell>(cell->dataObject).value_or(nullptr);
                if (data)
                {
                    data->ProfileClicked();
                }
            };

            auto clickableImage = image ? image->GetComponent<BSML::ClickableImage*>() : nullptr;
            if (clickableImage)
            {
                clickableImage->onClick += clickHandler;
            }

            auto nameText = FindTaggedText(cell, "row-title");
            auto clickableText = nameText ? nameText->GetComponent<BSML::ClickableText*>() : nullptr;
            if (clickableText)
            {
                clickableText->onClick += clickHandler;
            }
        }
    }

    void CompetePlayerCell::ProfileClicked()
    {
        if (player.playerId.empty() || player.isBot)
        {
            return;
        }

        if (_profileClicked)
        {
            _profileClicked(player.playerId, player.DisplayName());
        }
    }
}
