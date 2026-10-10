#pragma once

#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Protocol/LudusProto.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Packets
{
    // TSession is always ILudusSessionPacketContext in practice; the C# ILudusPacketSession
    // constraint (LastReceivedSequence get/set) becomes duck typing here
    template <typename TSession>
    struct ILudusEnvelopeHandler
    {
        virtual ~ILudusEnvelopeHandler() = default;
        virtual Protocol::LudusEnvelopeType Type() const = 0;
        virtual void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) = 0;
    };

    template <typename TSession>
    class LudusPacketDispatcher
    {
      public:
        // clock must stay rooted by the owning il2cpp service for the dispatcher's lifetime
        LudusPacketDispatcher(std::vector<std::unique_ptr<ILudusEnvelopeHandler<TSession>>> handlers, Core::Timing::SnoreSaberClock* clock) : _clock(clock)
        {
            for (auto& handler : handlers)
            {
                auto type = handler->Type();
                _handlers[type] = std::move(handler);
            }
        }

        void Handle(TSession& session, const std::vector<uint8_t>& bytes)
        {
            auto envelope = Protocol::LudusProto::Decode(bytes.data(), bytes.size());
            if (!envelope)
            {
                return;
            }

            _clock->RecordLudusServerTime(envelope->ServerTimeUnixMs);
            if (envelope->Sequence > session.LastReceivedSequence())
            {
                session.SetLastReceivedSequence(envelope->Sequence);
            }

            auto it = _handlers.find(envelope->Type);
            if (it != _handlers.end())
            {
                it->second->Handle(session, *envelope);
            }
        }

      private:
        std::unordered_map<Protocol::LudusEnvelopeType, std::unique_ptr<ILudusEnvelopeHandler<TSession>>> _handlers;
        Core::Timing::SnoreSaberClock* _clock;
    };
}
