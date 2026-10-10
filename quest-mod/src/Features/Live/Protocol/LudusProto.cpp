#include "Features/Live/Protocol/LudusProto.hpp"

#include "logging.hpp"

#include <array>
#include <random>
#include <utility>

namespace SnoreSaber::Features::Live::Protocol::LudusProto
{
    namespace
    {
        constexpr uint32_t ProtocolVersion = 1;

        // uuid v4 formatted like Guid.ToString("N") on pc: 32 lowercase hex chars.
        // pure c++ so encodes can run off the il2cpp main thread (deferred sends).
        std::string GenerateMessageId()
        {
            thread_local std::mt19937_64 rng = [] {
                std::random_device seedSource;
                std::seed_seq seed{seedSource(), seedSource(), seedSource(), seedSource()};
                return std::mt19937_64(seed);
            }();

            std::array<uint8_t, 16> bytes{};
            for (size_t offset = 0; offset < bytes.size(); offset += 8)
            {
                uint64_t chunk = rng();
                for (size_t index = 0; index < 8; index++)
                    bytes[offset + index] = uint8_t(chunk >> (index * 8));
            }

            bytes[6] = (bytes[6] & 0x0F) | 0x40;
            bytes[8] = (bytes[8] & 0x3F) | 0x80;

            static constexpr char hex[] = "0123456789abcdef";
            std::string messageId;
            messageId.reserve(32);
            for (uint8_t byte : bytes)
            {
                messageId.push_back(hex[byte >> 4]);
                messageId.push_back(hex[byte & 0x0F]);
            }

            return messageId;
        }

        std::vector<uint8_t> Encode(uint64_t sequence, const std::string& connectionId, int64_t clientTimeUnixMs, V1::LudusEnvelope envelope)
        {
            envelope.ProtocolVersion = ProtocolVersion;
            envelope.MessageId = GenerateMessageId();
            envelope.ConnectionId = connectionId;
            envelope.Sequence = sequence;
            envelope.ClientTimeUnixMs = clientTimeUnixMs;
            return V1::Serialize(envelope);
        }
    } // namespace

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
        uint64_t sequence)
    {
        V1::ConnectRequest connect;
        connect.AuthToken = authToken;
        connect.SessionId = sessionId;
        connect.SessionKey = sessionKey;
        connect.TournamentId = tournamentId;
        connect.PlayerId = playerId;
        connect.Platform = platform;
        connect.ClientType = V1::LudusClientType::Player;
        connect.GameVersion = gameVersion;
        connect.ClientVersion = clientVersion;
        connect.Mods = mods;
        connect.InitialRoomContext = initialRoomContext;
        connect.PublicLivePresenceOptOut = publicLivePresenceOptOut;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(connect);
        return Encode(sequence, "", clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeJoinRoom(
        const std::string& matchId,
        const std::string& roomId,
        const std::vector<V1::LiveMod>& mods,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::JoinRoomRequest join;
        join.MatchId = matchId;
        join.RoomId = roomId;
        join.Mods = mods;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(join);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeSetRoomContext(
        V1::LudusRoomContextType roomContext,
        const std::string& tournamentId,
        const std::vector<V1::LiveMod>& mods,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::SetRoomContextRequest request;
        request.RoomContext = roomContext;
        request.TournamentId = tournamentId;
        request.Mods = mods;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(request);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeSetClientType(
        V1::LudusClientType clientType,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::SetClientTypeRequest request;
        request.ClientType = clientType;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(request);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeLeaveRoom(
        const std::string& matchId,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::LeaveRoomRequest leave;
        leave.MatchId = matchId;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(leave);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodePresence(
        V1::LudusPlayState playState,
        V1::LudusDownloadState downloadState,
        const std::string& currentRoomId,
        const std::string& currentMapHash,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::PresenceUpdate presence;
        presence.PlayState = playState;
        presence.DownloadState = downloadState;
        presence.CurrentRoomId = currentRoomId;
        presence.CurrentMapHash = currentMapHash;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(presence);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeReplayPacket(
        V1::ReplayStreamPacket packet,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        packet.ConnectionId = connectionId;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(packet);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeReadyState(
        const std::string& matchId,
        bool ready,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::ReadyStateUpdate update;
        update.MatchId = matchId;
        update.ReadyState = ready ? V1::LudusReadyState::Ready : V1::LudusReadyState::NotReady;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(update);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeDownloadState(
        const std::string& matchId,
        V1::LudusDownloadState state,
        const std::string& errorMessage,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::DownloadStateUpdate update;
        update.MatchId = matchId;
        update.DownloadState = state;
        update.ErrorMessage = errorMessage;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(update);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodePromptResponse(
        const std::string& commandId,
        const std::string& matchId,
        const std::string& playerId,
        bool accepted,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::PromptResponse response;
        response.CommandId = commandId;
        response.MatchId = matchId;
        response.PlayerId = playerId;
        response.Accepted = accepted;
        response.RespondedAtUnixMs = clientTimeUnixMs;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(response);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeChatMessage(
        const std::string& matchId,
        const std::string& text,
        const std::string& senderDisplayName,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::LiveChatMessageRequest request;
        request.MatchId = matchId;
        request.Text = text;
        request.SenderDisplayName = senderDisplayName;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(request);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::vector<uint8_t> EncodeHeartbeat(
        uint64_t lastReceivedSequence,
        int64_t clientTimeUnixMs,
        uint64_t sequence,
        const std::string& connectionId)
    {
        V1::Heartbeat heartbeat;
        heartbeat.LastReceivedSequence = lastReceivedSequence;

        V1::LudusEnvelope envelope;
        envelope.Body = std::move(heartbeat);
        return Encode(sequence, connectionId, clientTimeUnixMs, std::move(envelope));
    }

    std::optional<DecodedLudusEnvelope> Decode(const uint8_t* data, size_t size)
    {
        V1::LudusEnvelope frame;
        if (!V1::Deserialize(frame, data, size))
        {
            WARN("Failed to parse ludus protobuf frame");
            return std::nullopt;
        }

        DecodedLudusEnvelope envelope;
        envelope.Sequence = frame.Sequence;
        envelope.ServerTimeUnixMs = frame.ServerTimeUnixMs;

        switch (frame.BodyCase())
        {
            case V1::LudusEnvelopeBodyCase::ConnectAccepted:
            {
                const auto& accepted = std::get<V1::ConnectAccepted>(frame.Body);
                envelope.Type = LudusEnvelopeType::ConnectAccepted;
                envelope.ConnectionId = accepted.ConnectionId;
                envelope.RoomContext = accepted.RoomContext;
                envelope.TournamentId = accepted.TournamentId;
                envelope.CurrentMatchId = accepted.CurrentMatchId;
                envelope.HeartbeatIntervalMs = int(accepted.HeartbeatIntervalMs);
                envelope.ClientType = accepted.ClientType;
                break;
            }
            case V1::LudusEnvelopeBodyCase::RoomContextUpdated:
            {
                const auto& updated = std::get<V1::RoomContextUpdated>(frame.Body);
                envelope.Type = LudusEnvelopeType::RoomContextUpdated;
                envelope.RoomContext = updated.RoomContext;
                envelope.TournamentId = updated.TournamentId;
                envelope.CurrentMatchId = updated.CurrentMatchId;
                envelope.ClientType = updated.ClientType;
                break;
            }
            case V1::LudusEnvelopeBodyCase::HeartbeatAck:
            {
                envelope.Type = LudusEnvelopeType::HeartbeatAck;
                break;
            }
            case V1::LudusEnvelopeBodyCase::ReconnectRequested:
            {
                const auto& reconnect = std::get<V1::ReconnectRequested>(frame.Body);
                envelope.Type = LudusEnvelopeType::ReconnectRequested;
                envelope.ReconnectWebSocketUrl = reconnect.WebsocketUrl;
                envelope.ReconnectReason = reconnect.Reason;
                envelope.ReconnectRetryAfterMs = int(reconnect.RetryAfterMs);
                break;
            }
            case V1::LudusEnvelopeBodyCase::RoomSnapshot:
            {
                envelope.Type = LudusEnvelopeType::RoomSnapshot;
                envelope.Rooms = std::move(std::get<V1::RoomSnapshot>(frame.Body).Rooms);
                break;
            }
            case V1::LudusEnvelopeBodyCase::ServerCommand:
            {
                envelope.Type = LudusEnvelopeType::ServerCommand;
                envelope.ServerCommand = std::move(std::get<V1::ServerCommand>(frame.Body));
                break;
            }
            case V1::LudusEnvelopeBodyCase::ChatMessage:
            {
                envelope.Type = LudusEnvelopeType::ChatMessage;
                envelope.ChatMessage = std::move(std::get<V1::LiveChatMessage>(frame.Body));
                break;
            }
            case V1::LudusEnvelopeBodyCase::ChatSnapshot:
            {
                envelope.Type = LudusEnvelopeType::ChatSnapshot;
                envelope.ChatSnapshot = std::move(std::get<V1::LiveChatSnapshot>(frame.Body));
                break;
            }
            case V1::LudusEnvelopeBodyCase::Error:
            {
                const auto& error = std::get<V1::ErrorResponse>(frame.Body);
                envelope.Type = LudusEnvelopeType::Error;
                envelope.ErrorCode = error.Code;
                envelope.ErrorMessage = error.Message;
                envelope.Retryable = error.Retryable;
                break;
            }
            default:
            {
                envelope.Type = LudusEnvelopeType::Unknown;
                break;
            }
        }

        return envelope;
    }
} // namespace SnoreSaber::Features::Live::Protocol::LudusProto
