#pragma once

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    enum class LeaderboardStatus
    {
        Unranked,
        Ranked,
        Qualified,
        Loved,
    };

    struct LeaderboardDetails
    {
        int id = 0;
        std::string songHash;
        std::string songName;
        std::string songSubName;
        std::string songAuthorName;
        std::string levelAuthorName;
        std::string coverImage;
        int difficulty = 0;
        std::string difficultyRaw;
        std::string gameMode;
        int maxScore = 0;
        int plays = 0;
        int dailyPlays = 0;
        std::string createdAt;
        std::optional<std::string> rankedAt;
        std::optional<std::string> qualifiedAt;
        std::optional<std::string> lovedAt;
        LeaderboardStatus status = LeaderboardStatus::Unranked;
        bool positiveModifiers = false;
        double stars = 0.0;
        int realmId = 0;
        std::string realmName;
    };
}
