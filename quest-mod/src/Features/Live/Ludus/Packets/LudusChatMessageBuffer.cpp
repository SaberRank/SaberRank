#include "Features/Live/Ludus/Packets/LudusChatMessageBuffer.hpp"

#include <algorithm>
#include <utility>

namespace SnoreSaber::Features::Live::Ludus::Packets
{
    std::vector<Domain::LiveChatEntry> LudusChatMessageBuffer::CurrentMessages() const
    {
        return _messages;
    }

    std::vector<Domain::LiveChatEntry> LudusChatMessageBuffer::MessagesFor(const std::string& matchId) const
    {
        std::vector<Domain::LiveChatEntry> result;
        if (matchId.empty())
        {
            return result;
        }

        for (const auto& message : _messages)
        {
            if (message.matchId == matchId)
            {
                result.push_back(message);
            }
        }
        return result;
    }

    bool LudusChatMessageBuffer::Apply(const ::SnoreSaber::Live::V1::LiveChatMessage& message, const std::string& currentMatchId)
    {
        auto entry = EntryForCurrentMatch(message, currentMatchId);
        if (!entry)
        {
            return false;
        }

        Upsert(std::move(*entry));
        SortAndTrim();
        return true;
    }

    void LudusChatMessageBuffer::Replace(const ::SnoreSaber::Live::V1::LiveChatSnapshot& snapshot, const std::string& currentMatchId)
    {
        if (currentMatchId.empty())
        {
            return;
        }

        for (const auto& message : snapshot.Messages)
        {
            auto entry = EntryForCurrentMatch(message, currentMatchId);
            if (entry)
            {
                Upsert(std::move(*entry));
            }
        }

        SortAndTrim();
    }

    bool LudusChatMessageBuffer::Clear()
    {
        if (_messages.empty())
        {
            return false;
        }

        _messages.clear();
        return true;
    }

    std::optional<Domain::LiveChatEntry> LudusChatMessageBuffer::EntryForCurrentMatch(const ::SnoreSaber::Live::V1::LiveChatMessage& message, const std::string& currentMatchId)
    {
        if (message.MatchId.empty() || message.MatchId != currentMatchId)
        {
            return std::nullopt;
        }

        return Domain::LiveChatEntry::FromProto(message);
    }

    void LudusChatMessageBuffer::Upsert(Domain::LiveChatEntry entry)
    {
        auto key = entry.Key();
        auto it = std::find_if(_messages.begin(), _messages.end(), [&key](const Domain::LiveChatEntry& item) { return item.Key() == key; });
        if (it != _messages.end())
        {
            *it = std::move(entry);
        }
        else
        {
            _messages.push_back(std::move(entry));
        }
    }

    void LudusChatMessageBuffer::SortAndTrim()
    {
        std::sort(_messages.begin(), _messages.end(), [](const Domain::LiveChatEntry& left, const Domain::LiveChatEntry& right) { return CompareEntries(left, right) < 0; });
        if (_messages.size() > MaxMessages)
        {
            _messages.erase(_messages.begin(), _messages.begin() + (_messages.size() - MaxMessages));
        }
    }

    int LudusChatMessageBuffer::CompareEntries(const Domain::LiveChatEntry& left, const Domain::LiveChatEntry& right)
    {
        // pc compares timestamps only when both are set, which is not a strict weak
        // ordering; treating unset (<=0) as 0 keeps the same priorities but stays
        // transitive, which std::sort requires
        int64_t leftCreatedAt = left.createdAtUnixMs > 0 ? left.createdAtUnixMs : 0;
        int64_t rightCreatedAt = right.createdAtUnixMs > 0 ? right.createdAtUnixMs : 0;
        if (leftCreatedAt != rightCreatedAt)
        {
            return leftCreatedAt < rightCreatedAt ? -1 : 1;
        }

        int matchComparison = left.matchId.compare(right.matchId);
        if (matchComparison != 0)
        {
            return matchComparison;
        }

        if (left.roomSequence != right.roomSequence)
        {
            return left.roomSequence < right.roomSequence ? -1 : 1;
        }

        return left.Key().compare(right.Key());
    }
}
