#include "Features/Live/Compete/Packets/Handlers/ReturnToMenuCommandHandler.hpp"

#include "logging.hpp"

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::ServerCommand;

    LudusCommandType ReturnToMenuCommandHandler::Type() const
    {
        return LudusCommandType::ReturnToMenu;
    }

    void ReturnToMenuCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        if (session.TryCancelPendingMapStart(command.MatchId))
        {
            session.NotifyStatusChanged("Map start cancelled.");
            INFO("Ludus: Pending live map start cancelled.");
            return;
        }

        if (session.GameplayControl()->TryStopMap(command.MatchId))
        {
            session.NotifyStatusChanged("Stopping map...");
            INFO("Ludus: Stopping live map.");
            return;
        }

        INFO("Ludus: Stop map requested, but no live map is active.");
    }
}
