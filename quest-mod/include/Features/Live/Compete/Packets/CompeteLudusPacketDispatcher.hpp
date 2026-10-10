#pragma once

#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Compete/Packets/ILudusServerCommandSession.hpp"
#include "Features/Live/Ludus/Packets/LudusChatMessageBuffer.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "Features/Live/Ludus/Services/ILudusSessionPacketContext.hpp"

namespace SnoreSaber::Features::Live::Compete::Packets::CompeteLudusPacketDispatcher
{
    // composes the default envelope handler set for the compete session; all pointers
    // are owned by (and must outlive with) the session service
    Ludus::Packets::LudusPacketDispatcher<Ludus::Services::ILudusSessionPacketContext> CreateDefault(
        ILudusServerCommandSession* commandSession,
        Ludus::Packets::LudusChatMessageBuffer* chatMessages,
        Core::Timing::SnoreSaberClock* clock);
}
