#pragma once

#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"
#include "Features/Live/Protocol/Generated/Common.hpp"
#include "Features/Live/Protocol/LudusProto.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    // envelope handlers talk to the session service through this; the delegate-bag
    // LudusSessionPacketContext is the only implementation (PC parity)
    struct ILudusSessionPacketContext
    {
        virtual ~ILudusSessionPacketContext() = default;

        virtual uint64_t LastReceivedSequence() const = 0;
        virtual void SetLastReceivedSequence(uint64_t value) = 0;
        virtual std::string ConnectionId() const = 0;
        virtual void SetConnectionId(const std::string& value) = 0;
        virtual float HeartbeatIntervalSeconds() const = 0;
        virtual void SetHeartbeatIntervalSeconds(float value) = 0;
        virtual ::SnoreSaber::Live::V1::LudusClientType ClientType() const = 0;
        virtual ::SnoreSaber::Live::V1::LudusRoomContextType RoomContext() const = 0;
        virtual std::string CurrentLudusMatchId() const = 0;
        virtual std::shared_ptr<Compete::Domain::CompeteRoom> PendingTournamentRoom() const = 0;

        virtual void ApplyClientContext(const Protocol::DecodedLudusEnvelope& envelope) = 0;
        virtual void CloseTournamentRoom() = 0;
        virtual void Disconnect() = 0;
        virtual void EnterTournamentRoom(const std::shared_ptr<Compete::Domain::CompeteRoom>& room) = 0;
        virtual void RejectPendingTournamentJoin(const std::string& message) = 0;
        virtual bool RequestAuthenticationRefresh() = 0;
        virtual void ScheduleNextHeartbeat() = 0;
        virtual void ScheduleReconnect(const std::string& reason, std::optional<float> delayOverrideSeconds) = 0;
        virtual void SendPresence(::SnoreSaber::Live::V1::LudusPlayState playState, ::SnoreSaber::Live::V1::LudusDownloadState downloadState, const std::string& currentMapHash) = 0;
        virtual void SetReconnectUrl(const std::string& url) = 0;
        virtual void NotifyChatMessagesChanged(const std::vector<Domain::LiveChatEntry>& messages) = 0;
        virtual void NotifyStatusChanged(const std::string& status) = 0;
    };
}
