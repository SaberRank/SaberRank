#pragma once

#include <string>

namespace SnoreSaber::Data
{
    struct GameAuthenticationRequest
    {
        int authType = 0;
        std::string playerId;
        std::string nonce;
        std::string friendIds;
        std::string playerName;
    };
}
