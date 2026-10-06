using SaberRank.Features.Live.Compete.Domain;
using SaberRank.Features.Live.Compete.Packets;
using SaberRank.Live.V1;
using System;

namespace SaberRank.Features.Live.Compete.Packets.Handlers {
    internal sealed class PromptCommandHandler : ILudusServerCommandHandler {
        public LudusCommandType Type => LudusCommandType.LudusCommandTypePrompt;

        public void Handle(ILudusServerCommandSession session, ServerCommand command) {
            session.NotifyPromptReceived(new CompeteOrganizerPrompt(
                string.IsNullOrEmpty(command.PromptTitle) ? "Organizer Prompt" : command.PromptTitle,
                command.PromptMessage ?? string.Empty,
                string.IsNullOrEmpty(command.PromptPrimaryText) ? "Confirm" : command.PromptPrimaryText,
                string.IsNullOrEmpty(command.PromptSecondaryText) ? "Dismiss" : command.PromptSecondaryText,
                command.CommandId,
                command.MatchId));
        }
    }
}
