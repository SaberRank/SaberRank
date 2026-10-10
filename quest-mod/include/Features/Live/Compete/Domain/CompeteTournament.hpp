#pragma once

#include <string>

namespace SnoreSaber::Features::Live::Compete::Domain
{
    struct CompeteTournament
    {
        std::string id;
        std::string name;
        std::string roomSummary;
    };
}
