#include "Features/Live/Compete/Packets/Handlers/CloseRoomCommandHandler.hpp"

#include "logging.hpp"

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::ServerCommand;

    LudusCommandType CloseRoomCommandHandler::Type() const
    {
        return LudusCommandType::CloseRoom;
    }

    void CloseRoomCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        auto room = session.TournamentRoom();
        if (!room)
        {
            return;
        }

        if (!command.MatchId.empty() && command.MatchId != room->id)
        {
            return;
        }

        session.NotifyStatusChanged("Room closed.");
        session.CloseTournamentRoom();
        INFO("Ludus: Closed room {}.", room->id);
    }
}
