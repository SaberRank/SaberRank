#include "Features/Players/Domain/LocalPlayerPanelState.hpp"

#include <utility>

namespace SnoreSaber::Data
{
    LocalPlayerPanelState LocalPlayerPanelState::Initial()
    {
        LocalPlayerPanelState state;
        state.globalRankingText = "<b><color=#FFDE1A>Global Ranking: </color></b> Loading...";
        return state;
    }

    LocalPlayerPanelState LocalPlayerPanelState::Loading(const LocalPlayerPanelState& previous)
    {
        LocalPlayerPanelState state = previous;
        state.isLoaded = false;
        state.promptErrorText.clear();
        state.promptDismissTime = -1.0f;
        return state;
    }

    LocalPlayerPanelState LocalPlayerPanelState::WithPlayer(Player player, bool usesWilliumsPanel, bool usesDenyahPanel)
    {
        LocalPlayerPanelState state;
        state.hasPlayerProfile = true;
        state.usesWilliumsPanel = usesWilliumsPanel;
        state.usesDenyahPanel = usesDenyahPanel;
        state.player = std::move(player);
        return state;
    }

    LocalPlayerPanelState LocalPlayerPanelState::Message(std::string text)
    {
        LocalPlayerPanelState state;
        state.globalRankingText = std::move(text);
        return state;
    }

    LocalPlayerPanelState LocalPlayerPanelState::Unavailable()
    {
        return Message("<b><color=#FFDE1A>Global Ranking: </color></b>Unavailable");
    }

    LocalPlayerPanelState LocalPlayerPanelState::PromptError(const LocalPlayerPanelState& previous, std::string promptText, float dismissTime)
    {
        LocalPlayerPanelState state = previous;
        state.isLoaded = true;
        state.promptErrorText = std::move(promptText);
        state.promptDismissTime = dismissTime;
        return state;
    }
}
