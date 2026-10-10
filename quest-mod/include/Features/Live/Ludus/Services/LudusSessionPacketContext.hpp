#pragma once

#include "Features/Live/Ludus/Services/ILudusSessionPacketContext.hpp"

#include <functional>

namespace SnoreSaber::Features::Live::Ludus::Services
{
    // forwards every context call to delegates the session service wires to itself
    // (PC passes 22 positional lambdas; designated initializers are the safer spelling here)
    class LudusSessionPacketContext final : public ILudusSessionPacketContext
    {
      public:
        struct Delegates
        {
            std::function<uint64_t()> getLastReceivedSequence;
            std::function<void(uint64_t)> setLastReceivedSequence;
            std::function<std::string()> getConnectionId;
            std::function<void(const std::string&)> setConnectionId;
            std::function<float()> getHeartbeatIntervalSeconds;
            std::function<void(float)> setHeartbeatIntervalSeconds;
            std::function<::SnoreSaber::Live::V1::LudusClientType()> getClientType;
            std::function<::SnoreSaber::Live::V1::LudusRoomContextType()> getRoomContext;
            std::function<std::string()> getCurrentLudusMatchId;
            std::function<std::shared_ptr<Compete::Domain::CompeteRoom>()> getPendingTournamentRoom;
            std::function<void(const Protocol::DecodedLudusEnvelope&)> applyClientContext;
            std::function<void()> closeTournamentRoom;
            std::function<void()> disconnect;
            std::function<void(const std::shared_ptr<Compete::Domain::CompeteRoom>&)> enterTournamentRoom;
            std::function<void(const std::string&)> rejectPendingTournamentJoin;
            std::function<bool()> requestAuthenticationRefresh;
            std::function<void()> scheduleNextHeartbeat;
            std::function<void(const std::string&, std::optional<float>)> scheduleReconnect;
            std::function<void(::SnoreSaber::Live::V1::LudusPlayState, ::SnoreSaber::Live::V1::LudusDownloadState, const std::string&)> sendPresence;
            std::function<void(const std::string&)> setReconnectUrl;
            std::function<void(const std::vector<Domain::LiveChatEntry>&)> notifyChatMessagesChanged;
            std::function<void(const std::string&)> notifyStatusChanged;
        };

        explicit LudusSessionPacketContext(Delegates delegates) : _delegates(std::move(delegates)) {}

        uint64_t LastReceivedSequence() const override { return _delegates.getLastReceivedSequence(); }
        void SetLastReceivedSequence(uint64_t value) override { _delegates.setLastReceivedSequence(value); }
        std::string ConnectionId() const override { return _delegates.getConnectionId(); }
        void SetConnectionId(const std::string& value) override { _delegates.setConnectionId(value); }
        float HeartbeatIntervalSeconds() const override { return _delegates.getHeartbeatIntervalSeconds(); }
        void SetHeartbeatIntervalSeconds(float value) override { _delegates.setHeartbeatIntervalSeconds(value); }
        ::SnoreSaber::Live::V1::LudusClientType ClientType() const override { return _delegates.getClientType(); }
        ::SnoreSaber::Live::V1::LudusRoomContextType RoomContext() const override { return _delegates.getRoomContext(); }
        std::string CurrentLudusMatchId() const override { return _delegates.getCurrentLudusMatchId(); }
        std::shared_ptr<Compete::Domain::CompeteRoom> PendingTournamentRoom() const override { return _delegates.getPendingTournamentRoom(); }

        void ApplyClientContext(const Protocol::DecodedLudusEnvelope& envelope) override { _delegates.applyClientContext(envelope); }
        void CloseTournamentRoom() override { _delegates.closeTournamentRoom(); }
        void Disconnect() override { _delegates.disconnect(); }
        void EnterTournamentRoom(const std::shared_ptr<Compete::Domain::CompeteRoom>& room) override { _delegates.enterTournamentRoom(room); }
        void RejectPendingTournamentJoin(const std::string& message) override { _delegates.rejectPendingTournamentJoin(message); }
        bool RequestAuthenticationRefresh() override { return _delegates.requestAuthenticationRefresh(); }
        void ScheduleNextHeartbeat() override { _delegates.scheduleNextHeartbeat(); }
        void ScheduleReconnect(const std::string& reason, std::optional<float> delayOverrideSeconds) override { _delegates.scheduleReconnect(reason, delayOverrideSeconds); }
        void SendPresence(::SnoreSaber::Live::V1::LudusPlayState playState, ::SnoreSaber::Live::V1::LudusDownloadState downloadState, const std::string& currentMapHash) override { _delegates.sendPresence(playState, downloadState, currentMapHash); }
        void SetReconnectUrl(const std::string& url) override { _delegates.setReconnectUrl(url); }
        void NotifyChatMessagesChanged(const std::vector<Domain::LiveChatEntry>& messages) override { _delegates.notifyChatMessagesChanged(messages); }
        void NotifyStatusChanged(const std::string& status) override { _delegates.notifyStatusChanged(status); }

      private:
        Delegates _delegates;
    };
}
