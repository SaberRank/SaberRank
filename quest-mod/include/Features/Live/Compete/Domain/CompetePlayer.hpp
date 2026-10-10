#pragma once

#include <string>

namespace SnoreSaber::Features::Live::Compete::Domain
{
    struct CompetePlayer
    {
        std::string name;
        std::string status;
        std::string teamId;
        std::string rank;
        bool isLocalPlayer = false;
        std::string playerId;
        bool isBot = false;
        std::string avatarUrl;

        std::string DisplayName() const
        {
            return isBot ? name + " [BOT]" : name;
        }
    };
}
