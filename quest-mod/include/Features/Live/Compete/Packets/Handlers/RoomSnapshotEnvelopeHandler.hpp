#pragma once

#include "Features/Live/Compete/Packets/ILudusServerCommandSession.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "Features/Live/Ludus/Services/ILudusSessionPacketContext.hpp"

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    class RoomSnapshotEnvelopeHandler final : public Ludus::Packets::ILudusEnvelopeHandler<Ludus::Services::ILudusSessionPacketContext>
    {
      public:
        // command session is owned by the session service alongside this handler
        explicit RoomSnapshotEnvelopeHandler(ILudusServerCommandSession* commandSession) : _commandSession(commandSession) {}

        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::RoomSnapshot;
        }

        void Handle(Ludus::Services::ILudusSessionPacketContext& session, const Protocol::DecodedLudusEnvelope& envelope) override;

      private:
        ILudusServerCommandSession* _commandSession;
    };
}
