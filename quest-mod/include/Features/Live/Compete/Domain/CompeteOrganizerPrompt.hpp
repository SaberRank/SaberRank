#pragma once

#include <string>

namespace SnoreSaber::Features::Live::Compete::Domain
{
    struct CompeteOrganizerPrompt
    {
        std::string title;
        std::string message;
        std::string primaryText;
        std::string secondaryText;
        std::string commandId;
        std::string matchId;
    };
}
