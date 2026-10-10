#pragma once

#include <UnityEngine/Sprite.hpp>

#include <string>
#include <string_view>

namespace SnoreSaber::Core::Presentation::PlayerPresentation
{
    struct CrownDetails
    {
        std::string image;
        std::string description;
    };

    std::string GetLoginSuccessText(std::string_view playerId);
    bool UsesFurryFont(std::string_view playerId);
    bool UsesWilliumsPanel(std::string_view playerId);
    bool UsesDenyahPanel(std::string_view playerId);
    CrownDetails GetCrownDetails(std::string_view playerId);
    UnityEngine::Sprite* GetCrownSprite(std::string_view image);
}
