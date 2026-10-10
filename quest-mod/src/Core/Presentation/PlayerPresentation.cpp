#include "Core/Presentation/PlayerPresentation.hpp"

#include "assets.hpp"

#include <bsml/shared/Helpers/utilities.hpp>

namespace SnoreSaber::Core::Presentation::PlayerPresentation
{
    namespace
    {
        constexpr std::string_view Denyah = "76561198064659288";
        constexpr std::string_view CyanSnow = "76561198019856958";
        constexpr std::string_view Williums = "76561198182060577";
        constexpr std::string_view Umbranox = "76561198283584459";
        constexpr std::string_view Woops = "76561198077062414";
        constexpr std::string_view Jones = "76561198066901156";
        constexpr std::string_view Rain = "76561198066644109";
    }

    std::string GetLoginSuccessText(std::string_view playerId)
    {
        if (playerId == Denyah)
        {
            return "Wagwan piffting wots ur bbm pin?";
        }

        return "Successfully signed into SnoreSaber!";
    }

    bool UsesFurryFont(std::string_view playerId)
    {
        return playerId == CyanSnow;
    }

    bool UsesWilliumsPanel(std::string_view playerId)
    {
        return playerId == Williums;
    }

    bool UsesDenyahPanel(std::string_view playerId)
    {
        return playerId == Denyah;
    }

    CrownDetails GetCrownDetails(std::string_view playerId)
    {
        if (playerId == Woops)
        {
            return { "SnoreSaber.Resources.crown-bronze.png", "Beat Saber Invitational 3rd place" };
        }
        if (playerId == Jones)
        {
            return { "SnoreSaber.Resources.crown-silver.png", "Beat Saber Invitational 2nd place" };
        }
        if (playerId == Umbranox)
        {
            return { "SnoreSaber.Resources.crown-umby.png", "Owner of SnoreSaber" };
        }
        if (playerId == Rain)
        {
            return { "SnoreSaber.Resources.crown-rain.png", "Owner of Umbranox's heart" };
        }
        return {};
    }

    UnityEngine::Sprite* GetCrownSprite(std::string_view image)
    {
        if (image.ends_with("crown-bronze.png"))
        {
            return BSML::Utilities::LoadSpriteRaw(IncludedAssets::crown_bronze_png);
        }
        if (image.ends_with("crown-rain.png"))
        {
            return BSML::Utilities::LoadSpriteRaw(IncludedAssets::crown_rain_png);
        }
        if (image.ends_with("crown-silver.png"))
        {
            return BSML::Utilities::LoadSpriteRaw(IncludedAssets::crown_silver_png);
        }
        if (image.ends_with("crown-umby.png"))
        {
            return BSML::Utilities::LoadSpriteRaw(IncludedAssets::crown_umby_png);
        }
        return nullptr;
    }
}
