#pragma once

#include <string>

namespace SnoreSaber::Data
{
    struct ScoreStats
    {
        ScoreStats();

        long totalScore;
        long totalRankedScore;
        double averageRankedAccuracy;
        int totalPlayCount;
        int rankedPlayCount;
        int replaysWatched;
    };
}
