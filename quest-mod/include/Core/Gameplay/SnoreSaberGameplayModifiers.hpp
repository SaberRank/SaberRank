#pragma once

#include <GlobalNamespace/GameplayModifiers.hpp>

#include <string>
#include <vector>

namespace SnoreSaber::Core::Gameplay::SnoreSaberGameplayModifiers
{
    struct GameplayModifiersMap
    {
        GlobalNamespace::GameplayModifiers* gameplayModifiers = nullptr;
        double totalMultiplier = 1.0;
    };

    GameplayModifiersMap FromCodes(const std::vector<std::string>& modifiers, bool isPositiveModifiersEnabled);
    std::vector<std::string> ToCodeList(GlobalNamespace::GameplayModifiers* gameplayModifiers, float energy);
}
