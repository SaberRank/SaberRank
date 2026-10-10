#pragma once

#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"

#include <algorithm>

namespace SnoreSaber::Features::Live::Ludus::Packets::Handlers
{
    template <typename TSession>
    class ReconnectRequestedEnvelopeHandler final : public ILudusEnvelopeHandler<TSession>
    {
      public:
        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::ReconnectRequested;
        }

        void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            if (envelope.ReconnectWebSocketUrl.empty())
            {
                return;
            }

            session.SetReconnectUrl(envelope.ReconnectWebSocketUrl);
            session.ScheduleReconnect(
                envelope.ReconnectReason.empty() ? "redirected" : envelope.ReconnectReason,
                std::max(0.05f, envelope.ReconnectRetryAfterMs / 1000.0f));
        }
    };
}
