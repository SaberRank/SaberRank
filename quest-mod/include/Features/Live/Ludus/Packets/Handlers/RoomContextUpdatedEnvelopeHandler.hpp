#pragma once

#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "logging.hpp"

namespace SnoreSaber::Features::Live::Ludus::Packets::Handlers
{
    template <typename TSession>
    class RoomContextUpdatedEnvelopeHandler final : public ILudusEnvelopeHandler<TSession>
    {
      public:
        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::RoomContextUpdated;
        }

        void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            session.ApplyClientContext(envelope);
            INFO("Ludus: Room context changed to {} {} {}", static_cast<int>(session.ClientType()), static_cast<int>(session.RoomContext()), session.CurrentLudusMatchId());
        }
    };
}
