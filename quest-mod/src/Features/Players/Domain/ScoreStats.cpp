#include "Features/Players/Domain/ScoreStats.hpp"

namespace SnoreSaber::Data
{
    ScoreStats::ScoreStats()
    {
        totalScore = 0;
        totalRankedScore = 0;
        averageRankedAccuracy = 0.0;
        totalPlayCount = 0;
        rankedPlayCount = 0;
        replaysWatched = 0;
    }
}
