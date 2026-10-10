#pragma once

#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Compete/Domain/CompeteOrganizerPrompt.hpp"
#include "Features/Live/Protocol/LudusProto.hpp"
#include "Features/Players/Domain/GameSession.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Packets
{
    namespace V1 = ::SnoreSaber::Live::V1;

    class LudusPacketSender
    {
      public:
        using SendFn = std::function<void(std::vector<uint8_t>)>;
        using SendDeferredFn = std::function<bool(std::function<std::vector<uint8_t>()>)>;

        // clock must stay rooted by the owning il2cpp service for the sender's lifetime
        LudusPacketSender(SendFn send, SendDeferredFn sendDeferred, Core::Timing::SnoreSaberClock* clock);

        uint64_t lastReceivedSequence = 0;

        void ResetSequences();

        void Connect(
            const SnoreSaber::Data::GameSession& session,
            V1::LivePlayerPlatform platform,
            const std::string& gameVersion,
            const std::string& clientVersion,
            V1::LudusRoomContextType initialRoomContext,
            bool publicLivePresenceOptOut,
            const std::vector<V1::LiveMod>& mods);
        void Heartbeat(const std::string& connectionId);
        void SetRoomContext(V1::LudusRoomContextType roomContext, const std::string& tournamentId, const std::vector<V1::LiveMod>& mods, const std::string& connectionId);
        void SetClientType(V1::LudusClientType clientType, const std::string& connectionId);
        void JoinRoom(const std::string& matchId, const std::vector<V1::LiveMod>& mods, const std::string& connectionId);
        void ReadyState(const std::string& matchId, bool ready, const std::string& connectionId);
        void DownloadState(const std::string& matchId, V1::LudusDownloadState state, const std::string& errorMessage, const std::string& connectionId);
        void PromptResponse(const Compete::Domain::CompeteOrganizerPrompt& prompt, const std::string& matchId, const std::string& playerId, bool accepted, const std::string& connectionId);
        void ChatMessage(const std::string& matchId, const std::string& text, const std::string& senderDisplayName, const std::string& connectionId);
        void Presence(V1::LudusPlayState playState, V1::LudusDownloadState downloadState, const std::string& currentMatchId, const std::string& currentMapHash, const std::string& connectionId);
        bool ReplayPacket(V1::ReplayStreamPacket packet, const std::string& connectionId);

      private:
        uint64_t NextSequence();
        int64_t NowUnixMs();

        SendFn _send;
        SendDeferredFn _sendDeferred;
        Core::Timing::SnoreSaberClock* _clock;
        uint64_t _outgoingSequence = 1;
    };
}
