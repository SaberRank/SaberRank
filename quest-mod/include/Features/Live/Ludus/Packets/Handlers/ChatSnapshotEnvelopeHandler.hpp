#pragma once

#include "Features/Live/Ludus/Packets/LudusChatMessageBuffer.hpp"
#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"

namespace SnoreSaber::Features::Live::Ludus::Packets::Handlers
{
    template <typename TSession>
    class ChatSnapshotEnvelopeHandler final : public ILudusEnvelopeHandler<TSession>
    {
      public:
        explicit ChatSnapshotEnvelopeHandler(LudusChatMessageBuffer* messages) : _messages(messages) {}

        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::ChatSnapshot;
        }

        void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            if (envelope.ChatSnapshot)
            {
                _messages->Replace(*envelope.ChatSnapshot, session.CurrentLudusMatchId());
            }

            session.NotifyChatMessagesChanged(_messages->MessagesFor(session.CurrentLudusMatchId()));
        }

      private:
        LudusChatMessageBuffer* _messages;
    };
}
