#pragma once

#include "Features/Live/Compete/Packets/LudusServerCommandDispatcher.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "Features/Live/Ludus/Services/ILudusSessionPacketContext.hpp"

#include <utility>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    class ServerCommandEnvelopeHandler final : public Ludus::Packets::ILudusEnvelopeHandler<Ludus::Services::ILudusSessionPacketContext>
    {
      public:
        // command session is owned by the session service alongside this handler
        ServerCommandEnvelopeHandler(LudusServerCommandDispatcher commandDispatcher, ILudusServerCommandSession* commandSession)
            : _commandDispatcher(std::move(commandDispatcher)), _commandSession(commandSession) {}

        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::ServerCommand;
        }

        void Handle(Ludus::Services::ILudusSessionPacketContext& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            _commandDispatcher.Handle(*_commandSession, envelope.ServerCommand);
        }

      private:
        LudusServerCommandDispatcher _commandDispatcher;
        ILudusServerCommandSession* _commandSession;
    };
}
