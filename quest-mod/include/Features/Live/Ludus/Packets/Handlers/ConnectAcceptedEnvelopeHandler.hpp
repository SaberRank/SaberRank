#pragma once

#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "logging.hpp"

namespace SnoreSaber::Features::Live::Ludus::Packets::Handlers
{
    template <typename TSession>
    class ConnectAcceptedEnvelopeHandler final : public ILudusEnvelopeHandler<TSession>
    {
      public:
        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::ConnectAccepted;
        }

        void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            session.SetConnectionId(envelope.ConnectionId);
            if (envelope.HeartbeatIntervalMs > 0)
            {
                session.SetHeartbeatIntervalSeconds(envelope.HeartbeatIntervalMs / 1000.0f);
            }

            session.ScheduleNextHeartbeat();
            session.ApplyClientContext(envelope);
            INFO("Ludus: Connected as {} {} in {} {}", session.ConnectionId(), static_cast<int>(session.ClientType()), static_cast<int>(envelope.RoomContext), session.CurrentLudusMatchId());
            session.SendPresence(::SnoreSaber::Live::V1::LudusPlayState::InMenus, ::SnoreSaber::Live::V1::LudusDownloadState::None, std::string());
            if (auto pendingRoom = session.PendingTournamentRoom())
            {
                session.EnterTournamentRoom(pendingRoom);
            }
        }
    };
}
