#include "Features/Live/Compete/Packets/Handlers/PromptCommandHandler.hpp"

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::ServerCommand;

    LudusCommandType PromptCommandHandler::Type() const
    {
        return LudusCommandType::Prompt;
    }

    void PromptCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        session.NotifyPromptReceived(Domain::CompeteOrganizerPrompt{
            .title = command.PromptTitle.empty() ? "Organizer Prompt" : command.PromptTitle,
            .message = command.PromptMessage,
            .primaryText = command.PromptPrimaryText.empty() ? "Confirm" : command.PromptPrimaryText,
            .secondaryText = command.PromptSecondaryText.empty() ? "Dismiss" : command.PromptSecondaryText,
            .commandId = command.CommandId,
            .matchId = command.MatchId});
    }
}
