#pragma once

#include "Features/Live/Ludus/Packets/LudusChatMessageBuffer.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"

namespace SnoreSaber::Features::Live::Ludus::Packets::Handlers
{
    template <typename TSession>
    class ChatMessageEnvelopeHandler final : public ILudusEnvelopeHandler<TSession>
    {
      public:
        // buffer is owned by the session service alongside this handler
        explicit ChatMessageEnvelopeHandler(LudusChatMessageBuffer* messages) : _messages(messages) {}

        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::ChatMessage;
        }

        void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            if (!envelope.ChatMessage)
            {
                return;
            }

            if (_messages->Apply(*envelope.ChatMessage, session.CurrentLudusMatchId()))
            {
                session.NotifyChatMessagesChanged(_messages->MessagesFor(session.CurrentLudusMatchId()));
            }
        }

      private:
        LudusChatMessageBuffer* _messages;
    };
}
