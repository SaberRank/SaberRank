#pragma once

#include "Features/Live/Protocol/Generated/Chat.hpp"

#include <cstdint>
#include <string>

namespace SnoreSaber::Features::Live::Ludus::Domain
{
    struct LiveChatEntry
    {
        std::string messageId;
        std::string matchId;
        std::string senderName;
        std::string senderPlayerId;
        ::SnoreSaber::Live::V1::LiveChatMessageKind kind = ::SnoreSaber::Live::V1::LiveChatMessageKind::Unspecified;
        std::string text;
        int64_t createdAtUnixMs = 0;
        uint64_t roomSequence = 0;

        bool IsChat() const
        {
            return kind == ::SnoreSaber::Live::V1::LiveChatMessageKind::Chat;
        }

        std::string Key() const
        {
            return messageId.empty() ? matchId + ":" + std::to_string(roomSequence) : messageId;
        }

        std::string DisplayTime() const;

        static LiveChatEntry FromProto(const ::SnoreSaber::Live::V1::LiveChatMessage& message);
    };
}
