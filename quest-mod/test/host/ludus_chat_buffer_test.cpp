// host-side checks for the ludus chat message buffer

#include "Features/Live/Ludus/Packets/LudusChatMessageBuffer.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

using ScoreSaber::Features::Live::Ludus::Packets::LudusChatMessageBuffer;
namespace V1 = ScoreSaber::Live::V1;

namespace
{
    [[noreturn]] void Fail(const std::string& message)
    {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
        std::exit(1);
    }

    void Require(bool condition, const std::string& message)
    {
        if (!condition)
        {
            Fail(message);
        }
    }

    V1::LiveChatMessage Message(const std::string& messageId, const std::string& matchId, int64_t createdAtUnixMs, uint64_t roomSequence, const std::string& text = "hi")
    {
        V1::LiveChatMessage message;
        message.MessageId = messageId;
        message.MatchId = matchId;
        message.SenderDisplayName = "sender";
        message.Kind = V1::LiveChatMessageKind::Chat;
        message.Text = text;
        message.CreatedAtUnixMs = createdAtUnixMs;
        message.RoomSequence = roomSequence;
        return message;
    }

    void TestApplyFiltersByMatch()
    {
        LudusChatMessageBuffer buffer;
        Require(!buffer.Apply(Message("m1", "other", 100, 1), "match"), "apply rejects other match");
        Require(!buffer.Apply(Message("m2", "", 100, 1), "match"), "apply rejects empty message match id");
        Require(buffer.Apply(Message("m3", "match", 100, 1), "match"), "apply accepts current match");
        Require(buffer.MessagesFor("match").size() == 1, "one message stored");
        Require(buffer.MessagesFor("").empty(), "empty match id yields nothing");
        Require(buffer.MessagesFor("other").empty(), "other match yields nothing");
    }

    void TestUpsertReplacesByKey()
    {
        LudusChatMessageBuffer buffer;
        buffer.Apply(Message("m1", "match", 100, 1, "first"), "match");
        buffer.Apply(Message("m1", "match", 100, 1, "edited"), "match");
        auto messages = buffer.MessagesFor("match");
        Require(messages.size() == 1, "upsert replaced instead of appending");
        Require(messages[0].text == "edited", "latest text won");

        // messages without an id key on match:sequence
        buffer.Apply(Message("", "match", 100, 7, "seq"), "match");
        buffer.Apply(Message("", "match", 100, 7, "seq edited"), "match");
        messages = buffer.MessagesFor("match");
        Require(messages.size() == 2, "sequence-keyed upsert replaced");
        Require(messages[1].text == "seq edited", "sequence-keyed latest text won");
    }

    void TestOrdering()
    {
        LudusChatMessageBuffer buffer;
        buffer.Apply(Message("late", "match", 300, 3), "match");
        buffer.Apply(Message("early", "match", 100, 9), "match");
        // no timestamps fall back to sequence ordering after timestamped entries compare equal-timestamp
        buffer.Apply(Message("seq2", "match", 0, 2), "match");
        buffer.Apply(Message("seq1", "match", 0, 1), "match");
        auto messages = buffer.MessagesFor("match");
        Require(messages.size() == 4, "four messages stored");
        Require(messages[0].messageId == "seq1" && messages[1].messageId == "seq2",
                "zero-timestamp entries ordered by sequence");
        Require(messages[2].messageId == "early" && messages[3].messageId == "late",
                "timestamped entries ordered by created-at");
    }

    void TestSnapshotReplaceAndTrim()
    {
        LudusChatMessageBuffer buffer;
        buffer.Apply(Message("keep", "match", 1, 1), "match");

        V1::LiveChatSnapshot snapshot;
        snapshot.MatchId = "match";
        for (int i = 0; i < 250; i++)
        {
            snapshot.Messages.push_back(Message("s" + std::to_string(i), "match", 1000 + i, static_cast<uint64_t>(i)));
        }
        snapshot.Messages.push_back(Message("foreign", "other", 5000, 1));

        buffer.Replace(snapshot, "match");
        auto messages = buffer.MessagesFor("match");
        Require(messages.size() == 200, "buffer trimmed to 200");
        // 251 entries ("keep" + s0..s249) trimmed by 51 from the oldest end
        Require(messages.front().messageId == "s50", "oldest entries trimmed first");
        Require(messages.back().messageId == "s249", "newest entry kept");
        Require(buffer.MessagesFor("other").empty(), "foreign match filtered out of snapshot");

        LudusChatMessageBuffer emptyTarget;
        emptyTarget.Replace(snapshot, "");
        Require(emptyTarget.CurrentMessages().empty(), "replace ignores empty current match");
    }

    void TestClear()
    {
        LudusChatMessageBuffer buffer;
        Require(!buffer.Clear(), "clear on empty buffer reports no change");
        buffer.Apply(Message("m1", "match", 100, 1), "match");
        Require(buffer.Clear(), "clear reports change");
        Require(buffer.CurrentMessages().empty(), "buffer emptied");
    }
}

int main()
{
    TestApplyFiltersByMatch();
    TestUpsertReplacesByKey();
    TestOrdering();
    TestSnapshotReplaceAndTrim();
    TestClear();
    std::printf("ludus_chat_buffer_test passed\n");
    return 0;
}
