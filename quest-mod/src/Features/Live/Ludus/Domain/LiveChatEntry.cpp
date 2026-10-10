#include "Features/Live/Ludus/Domain/LiveChatEntry.hpp"

#include <cstdio>
#include <ctime>

namespace SnoreSaber::Features::Live::Ludus::Domain
{
    std::string LiveChatEntry::DisplayTime() const
    {
        if (createdAtUnixMs <= 0)
        {
            return "--:--";
        }

        time_t seconds = static_cast<time_t>(createdAtUnixMs / 1000);
        tm local{};
        if (!localtime_r(&seconds, &local))
        {
            return "--:--";
        }

        char buffer[8];
        std::snprintf(buffer, sizeof(buffer), "%02d:%02d", local.tm_hour, local.tm_min);
        return buffer;
    }

    LiveChatEntry LiveChatEntry::FromProto(const ::SnoreSaber::Live::V1::LiveChatMessage& message)
    {
        LiveChatEntry entry;
        entry.messageId = message.MessageId;
        entry.matchId = message.MatchId;
        entry.senderName = message.SenderDisplayName;
        entry.senderPlayerId = message.SenderPlayerId;
        entry.kind = message.Kind;
        entry.text = message.Text;
        entry.createdAtUnixMs = message.CreatedAtUnixMs;
        entry.roomSequence = message.RoomSequence;
        return entry;
    }
}
