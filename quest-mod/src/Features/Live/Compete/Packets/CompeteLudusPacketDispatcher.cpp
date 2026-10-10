#include "Features/Live/Compete/Packets/CompeteLudusPacketDispatcher.hpp"

#include "Features/Live/Compete/Packets/Handlers/RoomSnapshotEnvelopeHandler.hpp"
#include "Features/Live/Compete/Packets/Handlers/ServerCommandEnvelopeHandler.hpp"
#include "Features/Live/Compete/Packets/LudusServerCommandDispatcher.hpp"
#include "Features/Live/Ludus/Packets/Handlers/ChatMessageEnvelopeHandler.hpp"
#include "Features/Live/Ludus/Packets/Handlers/ChatSnapshotEnvelopeHandler.hpp"
#include "Features/Live/Ludus/Packets/Handlers/ConnectAcceptedEnvelopeHandler.hpp"
#include "Features/Live/Ludus/Packets/Handlers/ErrorEnvelopeHandler.hpp"
#include "Features/Live/Ludus/Packets/Handlers/ReconnectRequestedEnvelopeHandler.hpp"
#include "Features/Live/Ludus/Packets/Handlers/RoomContextUpdatedEnvelopeHandler.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace SnoreSaber::Features::Live::Compete::Packets::CompeteLudusPacketDispatcher
{
    using Context = Ludus::Services::ILudusSessionPacketContext;

    Ludus::Packets::LudusPacketDispatcher<Context> CreateDefault(
        ILudusServerCommandSession* commandSession,
        Ludus::Packets::LudusChatMessageBuffer* chatMessages,
        Core::Timing::SnoreSaberClock* clock)
    {
        std::vector<std::unique_ptr<Ludus::Packets::ILudusEnvelopeHandler<Context>>> handlers;
        handlers.push_back(std::make_unique<Ludus::Packets::Handlers::ConnectAcceptedEnvelopeHandler<Context>>());
        handlers.push_back(std::make_unique<Ludus::Packets::Handlers::RoomContextUpdatedEnvelopeHandler<Context>>());
        handlers.push_back(std::make_unique<Ludus::Packets::Handlers::ReconnectRequestedEnvelopeHandler<Context>>());
        handlers.push_back(std::make_unique<Handlers::RoomSnapshotEnvelopeHandler>(commandSession));
        handlers.push_back(std::make_unique<Handlers::ServerCommandEnvelopeHandler>(LudusServerCommandDispatcher::CreateDefault(), commandSession));
        handlers.push_back(std::make_unique<Ludus::Packets::Handlers::ChatMessageEnvelopeHandler<Context>>(chatMessages));
        handlers.push_back(std::make_unique<Ludus::Packets::Handlers::ChatSnapshotEnvelopeHandler<Context>>(chatMessages));
        handlers.push_back(std::make_unique<Ludus::Packets::Handlers::ErrorEnvelopeHandler<Context>>());
        return Ludus::Packets::LudusPacketDispatcher<Context>(std::move(handlers), clock);
    }
}
