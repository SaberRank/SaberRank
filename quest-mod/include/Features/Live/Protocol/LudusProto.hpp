#pragma once

#include "Features/Live/Protocol/Generated/Ludus.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Protocol
{
    namespace V1 = ::SnoreSaber::Live::V1;

    enum class LudusEnvelopeType
    {
        Unknown,
        ConnectAccepted,
        RoomContextUpdated,
        HeartbeatAck,
        ReconnectRequested,
        RoomSnapshot,
        ServerCommand,
        ChatMessage,
        ChatSnapshot,
        Error,
    };

    struct DecodedLudusEnvelope
    {
        LudusEnvelopeType Type = LudusEnvelopeType::Unknown;
        uint64_t Sequence = 0;
        std::string ConnectionId;
        V1::LudusRoomContextType RoomContext{};
        std::string TournamentId;
        std::string CurrentMatchId;
        int64_t ServerTimeUnixMs = 0;
        int HeartbeatIntervalMs = 0;
        V1::LudusClientType ClientType = V1::LudusClientType::Player;
        std::string ReconnectWebSocketUrl;
        std::string ReconnectReason;
        int ReconnectRetryAfterMs = 0;
        std::vector<V1::LiveMatchRoomState> Rooms;
        std::optional<V1::ServerCommand> ServerCommand;
        std::optional<V1::LiveChatMessage> ChatMessage;
        std::optional<V1::LiveChatSnapshot> ChatSnapshot;
        std::string ErrorCode;
        std::string ErrorMessage;
        bool Retryable = false;
    };

    namespace LudusProto
    {
        std::vector<uint8_t> EncodeConnect(
            const std::string& authToken,
            const std::string& sessionId,
            const std::string& sessionKey,
            const std::string& tournamentId,
            const std::string& playerId,
            V1::LivePlayerPlatform platform,
            const std::string& gameVersion,
            const std::string& clientVersion,
            V1::LudusRoomContextType initialRoomContext,
            bool publicLivePresenceOptOut,
            const std::vector<V1::LiveMod>& mods,
            int64_t clientTimeUnixMs,
            uint64_t sequence);

        std::vector<uint8_t> EncodeJoinRoom(
            const std::string& matchId,
            const std::string& roomId,
            const std::vector<V1::LiveMod>& mods,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeSetRoomContext(
            V1::LudusRoomContextType roomContext,
            const std::string& tournamentId,
            const std::vector<V1::LiveMod>& mods,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeSetClientType(
            V1::LudusClientType clientType,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeLeaveRoom(
            const std::string& matchId,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodePresence(
            V1::LudusPlayState playState,
            V1::LudusDownloadState downloadState,
            const std::string& currentRoomId,
            const std::string& currentMapHash,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeReplayPacket(
            V1::ReplayStreamPacket packet,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeReadyState(
            const std::string& matchId,
            bool ready,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeDownloadState(
            const std::string& matchId,
            V1::LudusDownloadState state,
            const std::string& errorMessage,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodePromptResponse(
            const std::string& commandId,
            const std::string& matchId,
            const std::string& playerId,
            bool accepted,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeChatMessage(
            const std::string& matchId,
            const std::string& text,
            const std::string& senderDisplayName,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::vector<uint8_t> EncodeHeartbeat(
            uint64_t lastReceivedSequence,
            int64_t clientTimeUnixMs,
            uint64_t sequence,
            const std::string& connectionId);

        std::optional<DecodedLudusEnvelope> Decode(const uint8_t* data, size_t size);
    } // namespace LudusProto
} // namespace SnoreSaber::Features::Live::Protocol
