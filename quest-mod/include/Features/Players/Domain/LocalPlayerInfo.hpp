#pragma once

#include <string>

namespace SnoreSaber::Data
{
    struct LocalPlayerInfo
    {
        std::string nonce;
        std::string playerId;
        std::string playerName;
        std::string friends;
        int authType = 2;
    };
}
