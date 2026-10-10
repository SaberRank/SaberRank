#pragma once

#include "Features/Players/Domain/Player.hpp"

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    struct LocalPlayerPanelState
    {
        std::string globalRankingText;
        bool isLoaded = true;
        bool hasPlayerProfile = false;
        bool usesWilliumsPanel = false;
        bool usesDenyahPanel = false;
        std::string promptErrorText;
        float promptDismissTime = -1.0f;
        std::optional<Player> player;

        static LocalPlayerPanelState Initial();
        static LocalPlayerPanelState Loading(const LocalPlayerPanelState& previous);
        static LocalPlayerPanelState WithPlayer(Player player, bool usesWilliumsPanel, bool usesDenyahPanel);
        static LocalPlayerPanelState Message(std::string text);
        static LocalPlayerPanelState Unavailable();
        static LocalPlayerPanelState PromptError(const LocalPlayerPanelState& previous, std::string promptText, float dismissTime);
    };
}
