#pragma once

#include "Features/Players/Domain/Badge.hpp"
#include "Features/Players/Domain/ScoreStats.hpp"

#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Data
{
    struct Player
    {
        Player();
        Player(std::string _id);

        std::string id;
        std::u16string name;
        std::string plainName;
        std::string profilePicture;
        std::string country;
        double pp;
        int rank;
        int countryRank;
        std::string role;
        std::vector<Badge> badges;
        std::vector<int> histories;
        std::optional<::SnoreSaber::Data::ScoreStats> scoreStats;
        int permissions;
        bool banned;
        bool inactive;
    };
}
