#include "Features/Leaderboards/Domain/LeaderboardMap.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"

#include <utility>

namespace SnoreSaber::Data
{
    LeaderboardInfoMap::LeaderboardInfoMap(LeaderboardDetails leaderboard, GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey)
        : leaderboard(std::move(leaderboard)),
          beatmapLevel(beatmapLevel),
          beatmapKey(beatmapKey),
          songHash(SnoreSaber::Utils::SnoreSaberBeatmapKey::GetSongHash(beatmapKey))
    {
    }

    LeaderboardMap::LeaderboardMap(const LeaderboardSnapshot& leaderboard,
                                   GlobalNamespace::BeatmapLevel* beatmapLevel,
                                   GlobalNamespace::BeatmapKey beatmapKey,
                                   int maxScore,
                                   SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService)
        : leaderboardInfo(leaderboard.leaderboard, beatmapLevel, beatmapKey),
          playerScore(leaderboard.playerScore)
    {
        scores.reserve(leaderboard.scores.items.size());
        for (auto const& score : leaderboard.scores.items)
        {
            scores.emplace_back(score, leaderboardInfo, maxScore, replayStorageService);
        }
    }
}
