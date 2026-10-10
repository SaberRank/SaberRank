#include "Features/Live/Ludus/Packets/LudusPacketSender.hpp"

#include <utility>

namespace SnoreSaber::Features::Live::Ludus::Packets
{
    using Protocol::LudusProto::EncodeChatMessage;
    using Protocol::LudusProto::EncodeConnect;
    using Protocol::LudusProto::EncodeDownloadState;
    using Protocol::LudusProto::EncodeHeartbeat;
    using Protocol::LudusProto::EncodeJoinRoom;
    using Protocol::LudusProto::EncodePresence;
    using Protocol::LudusProto::EncodePromptResponse;
    using Protocol::LudusProto::EncodeReadyState;
    using Protocol::LudusProto::EncodeReplayPacket;
    using Protocol::LudusProto::EncodeSetClientType;
    using Protocol::LudusProto::EncodeSetRoomContext;

    LudusPacketSender::LudusPacketSender(SendFn send, SendDeferredFn sendDeferred, Core::Timing::SnoreSaberClock* clock)
        : _send(std::move(send)), _sendDeferred(std::move(sendDeferred)), _clock(clock)
    {
    }

    void LudusPacketSender::ResetSequences()
    {
        _outgoingSequence = 1;
        lastReceivedSequence = 0;
    }

    void LudusPacketSender::Connect(
        const SnoreSaber::Data::GameSession& session,
        V1::LivePlayerPlatform platform,
        const std::string& gameVersion,
        const std::string& clientVersion,
        V1::LudusRoomContextType initialRoomContext,
        bool publicLivePresenceOptOut,
        const std::vector<V1::LiveMod>& mods)
    {
        _send(EncodeConnect(
            std::string(),
            session.sessionId,
            session.sessionKey,
            std::string(),
            session.playerId,
            platform,
            gameVersion,
            clientVersion,
            initialRoomContext,
            publicLivePresenceOptOut,
            mods,
            NowUnixMs(),
            NextSequence()));
    }

    void LudusPacketSender::Heartbeat(const std::string& connectionId)
    {
        _send(EncodeHeartbeat(lastReceivedSequence, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::SetRoomContext(V1::LudusRoomContextType roomContext, const std::string& tournamentId, const std::vector<V1::LiveMod>& mods, const std::string& connectionId)
    {
        _send(EncodeSetRoomContext(roomContext, tournamentId, mods, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::SetClientType(V1::LudusClientType clientType, const std::string& connectionId)
    {
        _send(EncodeSetClientType(clientType, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::JoinRoom(const std::string& matchId, const std::vector<V1::LiveMod>& mods, const std::string& connectionId)
    {
        _send(EncodeJoinRoom(matchId, std::string(), mods, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::ReadyState(const std::string& matchId, bool ready, const std::string& connectionId)
    {
        _send(EncodeReadyState(matchId, ready, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::DownloadState(const std::string& matchId, V1::LudusDownloadState state, const std::string& errorMessage, const std::string& connectionId)
    {
        _send(EncodeDownloadState(matchId, state, errorMessage, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::PromptResponse(const Compete::Domain::CompeteOrganizerPrompt& prompt, const std::string& matchId, const std::string& playerId, bool accepted, const std::string& connectionId)
    {
        _send(EncodePromptResponse(prompt.commandId, matchId, playerId, accepted, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::ChatMessage(const std::string& matchId, const std::string& text, const std::string& senderDisplayName, const std::string& connectionId)
    {
        _send(EncodeChatMessage(matchId, text, senderDisplayName, NowUnixMs(), NextSequence(), connectionId));
    }

    void LudusPacketSender::Presence(V1::LudusPlayState playState, V1::LudusDownloadState downloadState, const std::string& currentMatchId, const std::string& currentMapHash, const std::string& connectionId)
    {
        _send(EncodePresence(playState, downloadState, currentMatchId, currentMapHash, NowUnixMs(), NextSequence(), connectionId));
    }

    bool LudusPacketSender::ReplayPacket(V1::ReplayStreamPacket packet, const std::string& connectionId)
    {
        uint64_t sequence = _outgoingSequence;
        int64_t clientTimeUnixMs = NowUnixMs();
        auto factory = [packet = std::move(packet), clientTimeUnixMs, sequence, connectionId]()
        {
            return EncodeReplayPacket(packet, clientTimeUnixMs, sequence, connectionId);
        };
        if (!_sendDeferred(std::move(factory)))
        {
            return false;
        }

        _outgoingSequence++;
        return true;
    }

    uint64_t LudusPacketSender::NextSequence()
    {
        return _outgoingSequence++;
    }

    int64_t LudusPacketSender::NowUnixMs()
    {
        return _clock->UnixTimeMilliseconds();
    }
}
