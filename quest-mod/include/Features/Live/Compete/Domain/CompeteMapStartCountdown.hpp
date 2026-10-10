#pragma once

#include <string>

namespace SnoreSaber::Features::Live::Compete::Domain
{
    struct CompeteMapStartCountdown
    {
        std::string matchId;
        int remainingSeconds = 0;
    };
}
