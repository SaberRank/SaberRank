// host-side round-trip coverage for the generated ludus live protocol codecs
// (Features/Live/Protocol/Generated) and the ProtoWire helper. golden byte
// vectors are hand-computed from the protobuf wire spec.

#include "Features/Live/Protocol/Generated/Ludus.hpp"
#include "Features/Live/Protocol/LudusProto.hpp"

#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <string_view>
#include <vector>

using namespace ScoreSaber::Live::V1;

namespace
{
    [[noreturn]] void Fail(std::string_view message)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }

    void Require(bool condition, std::string_view message)
    {
        if (!condition)
        {
            Fail(message);
        }
    }

    template <typename T>
    T RoundTrip(const T& value, std::string_view message)
    {
        auto bytes = Serialize(value);
        T parsed;
        Require(Deserialize(parsed, bytes.data(), bytes.size()), message);
        return parsed;
    }

    void TestGoldenBytes()
    {
        // LiveMod{Id:"a", Version:"b"} -> two length-delimited string fields
        LiveMod mod;
        mod.Id = "a";
        mod.Version = "b";
        Require(Serialize(mod) == std::vector<uint8_t>{0x0A, 0x01, 0x61, 0x12, 0x01, 0x62}, "LiveMod golden bytes");

        // Heartbeat{LastReceivedSequence:300} -> field 1 varint 300
        Heartbeat heartbeat;
        heartbeat.LastReceivedSequence = 300;
        Require(Serialize(heartbeat) == std::vector<uint8_t>{0x08, 0xAC, 0x02}, "Heartbeat golden bytes");

        // ReplayVector3{1.5, 0, -2} -> fixed32 fields 1 and 3, zero y skipped
        ReplayVector3 vector;
        vector.X = 1.5f;
        vector.Z = -2.0f;
        Require(Serialize(vector) == std::vector<uint8_t>{0x0D, 0x00, 0x00, 0xC0, 0x3F, 0x1D, 0x00, 0x00, 0x00, 0xC0},
                "ReplayVector3 golden bytes");

        // negative int64 sign-extends to a ten byte varint
        LudusEnvelope envelope;
        envelope.ClientTimeUnixMs = -1;
        Require(Serialize(envelope) == std::vector<uint8_t>{0x28, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01},
                "negative int64 golden bytes");

        // proto3 optional scalar set to its default value still serializes
        ReplayScoreEvent score;
        score.ImmediateMaxPossibleScore = 0;
        Require(Serialize(score) == std::vector<uint8_t>{0x18, 0x00}, "optional zero golden bytes");

        // empty message serializes to zero bytes and deserializes to defaults
        LudusEnvelope empty;
        Require(Serialize(empty).empty(), "default envelope serializes empty");
        Require(Deserialize(empty, nullptr, 0), "empty buffer deserializes");
        Require(empty.BodyCase() == LudusEnvelopeBodyCase::None, "empty buffer leaves oneof unset");
    }

    void TestOptionalScalarPresence()
    {
        ReplayScoreEvent score;
        score.Score = 115;
        score.TimeSeconds = 4.25f;
        score.ImmediateMaxPossibleScore = 0;

        auto parsed = RoundTrip(score, "score event round trip");
        Require(parsed.Score == 115, "score value");
        Require(parsed.TimeSeconds == 4.25f, "score time");
        Require(parsed.ImmediateMaxPossibleScore.has_value(), "optional zero survives round trip");
        Require(*parsed.ImmediateMaxPossibleScore == 0, "optional zero value");

        ReplayScoreEvent unset;
        unset.Score = 33;
        Require(!RoundTrip(unset, "unset optional round trip").ImmediateMaxPossibleScore.has_value(),
                "unset optional stays absent");

        LiveRoomReplayState state;
        state.PlayerId = "76561198000000000";
        state.Accuracy = 0.9987;
        state.Combo = 12;
        state.Completion = ReplayCompletion::Passed;

        auto parsedState = RoundTrip(state, "room replay state round trip");
        Require(parsedState.Accuracy.has_value() && *parsedState.Accuracy == 0.9987, "optional double");
        Require(parsedState.Combo.has_value() && *parsedState.Combo == 12, "optional uint32");
        Require(parsedState.Completion == ReplayCompletion::Passed, "completion enum");
    }

    void TestConnectEnvelopeRoundTrip()
    {
        LudusEnvelope envelope;
        envelope.ProtocolVersion = 1;
        envelope.MessageId = "abc123";
        envelope.Sequence = 7;
        envelope.ClientTimeUnixMs = 1720000000123;

        ConnectRequest connect;
        connect.AuthToken = "token";
        connect.SessionId = "session";
        connect.PlayerId = "player";
        connect.Platform = LivePlayerPlatform::Oculus;
        connect.ClientType = LudusClientType::Player;
        connect.GameVersion = "1.40.8";
        connect.ClientVersion = "4.0.0";
        connect.Mods.push_back(LiveMod{.Id = "scoresaber", .Version = "4.0.0"});
        connect.Mods.push_back(LiveMod{.Id = "bsml", .Version = "3.0.0"});
        connect.RoleNames.push_back("player");
        connect.InitialRoomContext = LudusRoomContextType::Core;
        connect.PublicLivePresenceOptOut = true;
        envelope.Body = connect;

        auto parsed = RoundTrip(envelope, "connect envelope round trip");
        Require(parsed.ProtocolVersion == 1, "protocol version");
        Require(parsed.MessageId == "abc123", "message id");
        Require(parsed.Sequence == 7, "sequence");
        Require(parsed.ClientTimeUnixMs == 1720000000123, "client time");
        Require(parsed.BodyCase() == LudusEnvelopeBodyCase::ConnectRequest, "connect body case");

        const auto& parsedConnect = std::get<ConnectRequest>(parsed.Body);
        Require(parsedConnect.AuthToken == "token", "auth token");
        Require(parsedConnect.Platform == LivePlayerPlatform::Oculus, "platform enum");
        Require(parsedConnect.Mods.size() == 2 && parsedConnect.Mods[1].Id == "bsml", "repeated mods");
        Require(parsedConnect.RoleNames == std::vector<std::string>{"player"}, "repeated strings");
        Require(parsedConnect.InitialRoomContext == LudusRoomContextType::Core, "room context enum");
        Require(parsedConnect.PublicLivePresenceOptOut, "opt out flag");
    }

    void TestRoomSnapshotRoundTrip()
    {
        LudusEnvelope envelope;
        envelope.Sequence = 42;
        envelope.ServerTimeUnixMs = 1720000000456;

        RoomSnapshot snapshot;
        LiveMatchRoomState room;
        room.MatchId = "match-1";
        room.RoomId = "room-1";
        room.LoadedSong = true;
        room.LoadedSongHash = "DEADBEEF";
        room.PlayerIds = {"p1", "p2"};
        room.ViewerCount = 3;

        LiveRoomPlayerState player;
        player.PlayerId = "p1";
        player.PlayState = LudusPlayState::InGame;
        player.DownloadState = LudusDownloadState::Downloaded;
        player.ReadyState = LudusReadyState::Ready;
        room.PlayerStates.push_back(player);

        LiveRoomViewerState viewer;
        viewer.PlayerId = "caster";
        viewer.ClientType = LudusClientType::Caster;
        room.Viewers.push_back(viewer);

        snapshot.Rooms.push_back(room);
        snapshot.Rooms.push_back(LiveMatchRoomState{.MatchId = "match-2"});
        envelope.Body = snapshot;

        auto parsed = RoundTrip(envelope, "room snapshot round trip");
        Require(parsed.BodyCase() == LudusEnvelopeBodyCase::RoomSnapshot, "snapshot body case");

        const auto& parsedSnapshot = std::get<RoomSnapshot>(parsed.Body);
        Require(parsedSnapshot.Rooms.size() == 2, "room count");
        const auto& parsedRoom = parsedSnapshot.Rooms[0];
        Require(parsedRoom.LoadedSong && parsedRoom.LoadedSongHash == "DEADBEEF", "room song state");
        Require(parsedRoom.PlayerIds.size() == 2 && parsedRoom.PlayerIds[1] == "p2", "room player ids");
        Require(parsedRoom.PlayerStates.size() == 1 && parsedRoom.PlayerStates[0].PlayState == LudusPlayState::InGame,
                "nested player state");
        Require(parsedRoom.Viewers.size() == 1 && parsedRoom.Viewers[0].ClientType == LudusClientType::Caster,
                "nested viewer state");
        Require(parsedSnapshot.Rooms[1].MatchId == "match-2" && parsedSnapshot.Rooms[1].PlayerStates.empty(),
                "sparse second room");
    }

    void TestReplayPacketRoundTrip()
    {
        LudusEnvelope envelope;
        envelope.Sequence = 99;

        ReplayStreamPacket packet;
        packet.StreamId = "stream-1";
        packet.PlayerId = "p1";

        ReplayChunk chunk;
        ReplayCursor cursor;
        cursor.Sequence = 12;
        cursor.SongTimeMs = 34567;
        cursor.ClientTimeUnixMs = 1720000000789;
        chunk.Cursor = cursor;

        StreamReplayEventBatch batch;
        ReplayPoseFrame frame;
        frame.Head = ReplayPose{.Position = ReplayVector3{.X = 0.1f, .Y = 1.7f, .Z = -0.2f},
                                .Rotation = ReplayQuaternion{.W = 1.0f}};
        frame.Fps = 72;
        frame.TimeSeconds = 12.5f;
        batch.PoseFrames.push_back(frame);

        ReplayNoteEvent note;
        note.NoteId = ReplayNoteId{.TimeSeconds = 12.25f, .LineLayer = 1, .LineIndex = 2, .ColorType = 0, .CutDirection = 1};
        note.NoteId->ScoringType = 3;
        note.EventType = ReplayNoteEventType::GoodCut;
        note.SaberSpeed = 42.5f;
        note.TimeDeviation = -0.011f;
        batch.NoteEvents.push_back(note);

        ReplayNoteEvent miss;
        miss.EventType = ReplayNoteEventType::Miss;
        batch.NoteEvents.push_back(miss);

        batch.MinTimeSeconds = 12.0f;
        batch.MaxTimeSeconds = 13.0f;
        chunk.Events = batch;

        ReplayEventCounts counts;
        counts.PoseFrames = 900;
        counts.NoteEvents = 2;
        chunk.CumulativeEventCounts = counts;
        chunk.ReplayExtensions.push_back(ReplayExtension{.Id = "hsv", .Version = 1, .Payload = {0x01, 0x02, 0x03}});

        packet.Body = chunk;
        envelope.Body = packet;

        auto parsed = RoundTrip(envelope, "replay packet round trip");
        Require(parsed.BodyCase() == LudusEnvelopeBodyCase::ReplayPacket, "replay body case");

        const auto& parsedPacket = std::get<ReplayStreamPacket>(parsed.Body);
        Require(parsedPacket.StreamId == "stream-1", "stream id");
        Require(parsedPacket.BodyCase() == ReplayStreamPacketBodyCase::Chunk, "packet body case");

        const auto& parsedChunk = std::get<ReplayChunk>(parsedPacket.Body);
        Require(parsedChunk.Cursor.has_value() && parsedChunk.Cursor->SongTimeMs == 34567, "chunk cursor");
        Require(parsedChunk.Events.has_value(), "chunk events");
        Require(parsedChunk.Events->PoseFrames.size() == 1, "pose frame count");

        const auto& parsedFrame = parsedChunk.Events->PoseFrames[0];
        Require(parsedFrame.Head.has_value() && parsedFrame.Head->Position.has_value(), "pose head present");
        Require(parsedFrame.Head->Position->Y == 1.7f, "pose head y");
        Require(!parsedFrame.Left.has_value(), "unset pose stays absent");
        Require(parsedFrame.Fps == 72 && parsedFrame.TimeSeconds == 12.5f, "pose frame scalars");

        const auto& parsedNote = parsedChunk.Events->NoteEvents[0];
        Require(parsedNote.NoteId.has_value() && parsedNote.NoteId->LineIndex == 2, "note id");
        Require(parsedNote.NoteId->ScoringType.has_value() && *parsedNote.NoteId->ScoringType == 3, "note scoring type");
        Require(!parsedNote.NoteId->GameplayType.has_value(), "unset note optional");
        Require(parsedNote.EventType == ReplayNoteEventType::GoodCut, "note event type");
        Require(parsedNote.TimeDeviation.has_value() && *parsedNote.TimeDeviation == -0.011f, "note time deviation");
        Require(!parsedChunk.Events->NoteEvents[1].NoteId.has_value(), "sparse miss event");
        Require(parsedChunk.CumulativeEventCounts->PoseFrames == 900, "event counts");
        Require(parsedChunk.ReplayExtensions.size() == 1 && parsedChunk.ReplayExtensions[0].Payload.size() == 3,
                "extension bytes");
    }

    void TestPackedRepeatedVarints()
    {
        ReplayAck ack;
        ack.HighestContiguousSequence = 41;
        ack.MissingSequences = {1, 300, 70000};

        auto bytes = Serialize(ack);
        // field 2 must be packed: single length-delimited record
        Require(bytes == std::vector<uint8_t>{0x08, 0x29, 0x12, 0x06, 0x01, 0xAC, 0x02, 0xF0, 0xA2, 0x04},
                "packed missing sequences golden bytes");

        auto parsed = RoundTrip(ack, "ack round trip");
        Require(parsed.MissingSequences == std::vector<uint64_t>{1, 300, 70000}, "packed values");

        // non-packed encoding of the same field must decode too
        std::vector<uint8_t> unpacked = {0x10, 0x01, 0x10, 0xAC, 0x02};
        ReplayAck fromUnpacked;
        Require(Deserialize(fromUnpacked, unpacked.data(), unpacked.size()), "unpacked decode");
        Require(fromUnpacked.MissingSequences == std::vector<uint64_t>{1, 300}, "unpacked values");
    }

    void TestUnknownFieldsAreSkipped()
    {
        // Heartbeat{300} followed by unknown fields of every wire type:
        // field 99 varint, field 98 length-delimited "abc", field 97 fixed32, field 96 fixed64
        std::vector<uint8_t> bytes = {0x08, 0xAC, 0x02};
        bytes.insert(bytes.end(), {0x98, 0x06, 0x01});
        bytes.insert(bytes.end(), {0x92, 0x06, 0x03, 0x61, 0x62, 0x63});
        bytes.insert(bytes.end(), {0x8D, 0x06, 0x01, 0x02, 0x03, 0x04});
        bytes.insert(bytes.end(), {0x81, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08});

        Heartbeat heartbeat;
        Require(Deserialize(heartbeat, bytes.data(), bytes.size()), "unknown fields decode");
        Require(heartbeat.LastReceivedSequence == 300, "known field survives unknown neighbors");
    }

    void TestMalformedInputIsRejected()
    {
        auto rejects = [](std::initializer_list<uint8_t> bytes, std::string_view message) {
            std::vector<uint8_t> buffer(bytes);
            LudusEnvelope envelope;
            Require(!Deserialize(envelope, buffer.data(), buffer.size()), message);
        };

        rejects({0x08}, "truncated varint rejected");
        rejects({0x0A, 0x05, 0x61}, "truncated length-delimited rejected");
        rejects({0x0B}, "group wire type rejected");
        rejects({0x00}, "zero field number rejected");
        rejects({0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01}, "overlong varint rejected");

        // valid envelope with a corrupted (truncated) nested body
        std::vector<uint8_t> nested = {0x52, 0x04, 0x0A, 0xFF, 0x01, 0x61};
        LudusEnvelope envelope;
        Require(!Deserialize(envelope, nested.data(), nested.size()), "corrupt nested message rejected");
    }
    void TestLudusProtoEncode()
    {
        namespace Protocol = ScoreSaber::Features::Live::Protocol;
        namespace LudusProto = Protocol::LudusProto;

        std::vector<LiveMod> mods = {LiveMod{.Id = "scoresaber", .Version = "4.0.0"}};
        auto bytes = LudusProto::EncodeConnect(
            "token", "session", "key", "tournament", "player",
            LivePlayerPlatform::Oculus, "1.40.8", "4.0.0",
            LudusRoomContextType::Core, true, mods, 1720000000123, 1);

        LudusEnvelope frame;
        Require(Deserialize(frame, bytes.data(), bytes.size()), "encoded connect parses");
        Require(frame.ProtocolVersion == 1, "connect protocol version");
        Require(frame.Sequence == 1, "connect sequence");
        Require(frame.ClientTimeUnixMs == 1720000000123, "connect client time");
        Require(frame.ConnectionId.empty(), "connect has no connection id yet");
        Require(frame.MessageId.size() == 32, "message id is guid n format");
        for (char digit : frame.MessageId)
            Require(std::isxdigit(static_cast<unsigned char>(digit)) && !std::isupper(static_cast<unsigned char>(digit)),
                    "message id hex digits");
        Require(frame.BodyCase() == LudusEnvelopeBodyCase::ConnectRequest, "connect body");

        const auto& connect = std::get<ConnectRequest>(frame.Body);
        Require(connect.AuthToken == "token" && connect.SessionKey == "key", "connect credentials");
        Require(connect.ClientType == LudusClientType::Player, "connect forces player client type");
        Require(connect.Mods.size() == 1 && connect.Mods[0].Id == "scoresaber", "connect mods");
        Require(connect.PublicLivePresenceOptOut, "connect opt out");

        auto second = LudusProto::EncodeHeartbeat(5, 1720000000456, 2, "conn-1");
        LudusEnvelope heartbeatFrame;
        Require(Deserialize(heartbeatFrame, second.data(), second.size()), "encoded heartbeat parses");
        Require(heartbeatFrame.ConnectionId == "conn-1", "heartbeat connection id");
        Require(heartbeatFrame.MessageId != frame.MessageId, "message ids are unique");
        Require(std::get<Heartbeat>(heartbeatFrame.Body).LastReceivedSequence == 5, "heartbeat sequence");

        auto packetBytes = LudusProto::EncodeReplayPacket(
            ReplayStreamPacket{.StreamId = "stream-1"}, 1720000000789, 3, "conn-1");
        LudusEnvelope packetFrame;
        Require(Deserialize(packetFrame, packetBytes.data(), packetBytes.size()), "encoded replay packet parses");
        const auto& packet = std::get<ReplayStreamPacket>(packetFrame.Body);
        Require(packet.StreamId == "stream-1", "packet stream id");
        Require(packet.ConnectionId == "conn-1", "packet connection id stamped");

        auto readyBytes = LudusProto::EncodeReadyState("match-1", false, 1, 4, "conn-1");
        LudusEnvelope readyFrame;
        Require(Deserialize(readyFrame, readyBytes.data(), readyBytes.size()), "encoded ready state parses");
        Require(std::get<ReadyStateUpdate>(readyFrame.Body).ReadyState == LudusReadyState::NotReady,
                "ready false maps to not ready");
    }

    void TestLudusProtoDecode()
    {
        namespace Protocol = ScoreSaber::Features::Live::Protocol;
        namespace LudusProto = Protocol::LudusProto;

        LudusEnvelope frame;
        frame.Sequence = 10;
        frame.ServerTimeUnixMs = 1720000001000;
        ConnectAccepted accepted;
        accepted.ConnectionId = "conn-9";
        accepted.TournamentId = "tourney";
        accepted.CurrentMatchId = "match-3";
        accepted.HeartbeatIntervalMs = 15000;
        accepted.RoomContext = LudusRoomContextType::Tournament;
        accepted.ClientType = LudusClientType::Spectator;
        frame.Body = accepted;

        auto bytes = Serialize(frame);
        auto decoded = LudusProto::Decode(bytes.data(), bytes.size());
        Require(decoded.has_value(), "connect accepted decodes");
        Require(decoded->Type == Protocol::LudusEnvelopeType::ConnectAccepted, "decoded type");
        Require(decoded->Sequence == 10 && decoded->ServerTimeUnixMs == 1720000001000, "decoded envelope scalars");
        Require(decoded->ConnectionId == "conn-9" && decoded->CurrentMatchId == "match-3", "decoded ids");
        Require(decoded->HeartbeatIntervalMs == 15000, "decoded heartbeat interval");
        Require(decoded->RoomContext == LudusRoomContextType::Tournament, "decoded room context");
        Require(decoded->ClientType == LudusClientType::Spectator, "decoded client type");

        LudusEnvelope snapshotFrame;
        RoomSnapshot snapshot;
        snapshot.Rooms.push_back(LiveMatchRoomState{.MatchId = "match-1"});
        snapshotFrame.Body = snapshot;
        auto snapshotBytes = Serialize(snapshotFrame);
        auto decodedSnapshot = LudusProto::Decode(snapshotBytes.data(), snapshotBytes.size());
        Require(decodedSnapshot.has_value() && decodedSnapshot->Type == Protocol::LudusEnvelopeType::RoomSnapshot,
                "room snapshot decodes");
        Require(decodedSnapshot->Rooms.size() == 1 && decodedSnapshot->Rooms[0].MatchId == "match-1",
                "decoded rooms");

        LudusEnvelope errorFrame;
        ErrorResponse error;
        error.Code = "AUTH_EXPIRED";
        error.Message = "expired";
        error.Retryable = true;
        errorFrame.Body = error;
        auto errorBytes = Serialize(errorFrame);
        auto decodedError = LudusProto::Decode(errorBytes.data(), errorBytes.size());
        Require(decodedError.has_value() && decodedError->Type == Protocol::LudusEnvelopeType::Error, "error decodes");
        Require(decodedError->ErrorCode == "AUTH_EXPIRED" && decodedError->Retryable, "decoded error fields");

        // client-only body arriving from the server decodes as unknown, keeping envelope scalars
        LudusEnvelope unknownFrame;
        unknownFrame.Sequence = 77;
        unknownFrame.Body = Heartbeat{.LastReceivedSequence = 1};
        auto unknownBytes = Serialize(unknownFrame);
        auto decodedUnknown = LudusProto::Decode(unknownBytes.data(), unknownBytes.size());
        Require(decodedUnknown.has_value() && decodedUnknown->Type == Protocol::LudusEnvelopeType::Unknown,
                "unhandled body decodes as unknown");
        Require(decodedUnknown->Sequence == 77, "unknown keeps sequence");
        Require(decodedUnknown->ClientType == LudusClientType::Player, "decoded client type defaults to player");

        std::vector<uint8_t> garbage = {0x08};
        Require(!LudusProto::Decode(garbage.data(), garbage.size()).has_value(), "malformed frame decodes to nullopt");
    }
} // namespace

int main()
{
    TestGoldenBytes();
    TestOptionalScalarPresence();
    TestConnectEnvelopeRoundTrip();
    TestRoomSnapshotRoundTrip();
    TestReplayPacketRoundTrip();
    TestPackedRepeatedVarints();
    TestUnknownFieldsAreSkipped();
    TestMalformedInputIsRejected();
    TestLudusProtoEncode();
    TestLudusProtoDecode();

    std::cout << "live proto round-trip tests passed" << '\n';
    return 0;
}
