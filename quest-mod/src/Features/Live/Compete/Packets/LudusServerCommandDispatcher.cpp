#include "Features/Live/Compete/Packets/LudusServerCommandDispatcher.hpp"

#include "Features/Live/Compete/Packets/Handlers/CloseRoomCommandHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/CreateRoomCommandHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/FollowPlayerCommandHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/LoadSongCommandHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/PromptCommandHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/ReturnToMenuCommandHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/StartMapCommandHandler.hpp"
#include "logging.hpp"

#include <algorithm>
#include <utility>

namespace SnoreSaber::Features::Live::Compete::Packets
{
    using ::SnoreSaber::Live::V1::ServerCommand;

    LudusServerCommandDispatcher::LudusServerCommandDispatcher(std::vector<std::unique_ptr<ILudusServerCommandHandler>> handlers)
    {
        for (auto& handler : handlers)
        {
            auto type = handler->Type();
            _handlers[type] = std::move(handler);
        }
    }

    LudusServerCommandDispatcher LudusServerCommandDispatcher::CreateDefault()
    {
        std::vector<std::unique_ptr<ILudusServerCommandHandler>> handlers;
        handlers.push_back(std::make_unique<Handlers::CreateRoomCommandHandler>());
        handlers.push_back(std::make_unique<Handlers::CloseRoomCommandHandler>());
        handlers.push_back(std::make_unique<Handlers::PromptCommandHandler>());
        handlers.push_back(std::make_unique<Handlers::LoadSongCommandHandler>());
        handlers.push_back(std::make_unique<Handlers::StartMapCommandHandler>());
        handlers.push_back(std::make_unique<Handlers::ReturnToMenuCommandHandler>());
        handlers.push_back(std::make_unique<Handlers::FollowPlayerCommandHandler>());
        return LudusServerCommandDispatcher(std::move(handlers));
    }

    namespace
    {
        bool TargetsLocalPlayer(ILudusServerCommandSession& session, const ServerCommand& command)
        {
            if (command.TargetPlayerIds.empty())
            {
                return true;
            }

            auto localPlayerId = session.LocalPlayerId();
            return std::find(command.TargetPlayerIds.begin(), command.TargetPlayerIds.end(), localPlayerId) != command.TargetPlayerIds.end();
        }
    }

    void LudusServerCommandDispatcher::Handle(ILudusServerCommandSession& session, const std::optional<ServerCommand>& command)
    {
        if (!command)
        {
            return;
        }

        if (!TargetsLocalPlayer(session, *command))
        {
            SnoreSaber::Logging::Logger.debug("Ludus: Ignored server command {} for match {}.", static_cast<int>(command->Type), command->MatchId);
            return;
        }

        auto it = _handlers.find(command->Type);
        if (it != _handlers.end())
        {
            INFO("Ludus: Handling server command {} for match {}.", static_cast<int>(command->Type), command->MatchId);
            it->second->Handle(session, *command);
        }
        else
        {
            WARN("Ludus: No handler registered for server command {}.", static_cast<int>(command->Type));
        }
    }
}
