#pragma once

#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"
#include "Features/Live/Protocol/Generated/Chat.hpp"

#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Packets
{
    class LudusChatMessageBuffer
    {
      public:
        std::vector<Domain::LiveChatEntry> CurrentMessages() const;
        std::vector<Domain::LiveChatEntry> MessagesFor(const std::string& matchId) const;
        bool Apply(const ::SnoreSaber::Live::V1::LiveChatMessage& message, const std::string& currentMatchId);
        void Replace(const ::SnoreSaber::Live::V1::LiveChatSnapshot& snapshot, const std::string& currentMatchId);
        bool Clear();

      private:
        static constexpr size_t MaxMessages = 200;

        static std::optional<Domain::LiveChatEntry> EntryForCurrentMatch(const ::SnoreSaber::Live::V1::LiveChatMessage& message, const std::string& currentMatchId);
        void Upsert(Domain::LiveChatEntry entry);
        void SortAndTrim();
        static int CompareEntries(const Domain::LiveChatEntry& left, const Domain::LiveChatEntry& right);

        std::vector<Domain::LiveChatEntry> _messages;
    };
}
