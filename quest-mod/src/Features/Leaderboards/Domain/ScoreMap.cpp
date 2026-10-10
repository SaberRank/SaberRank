#include "Features/Leaderboards/Domain/ScoreMap.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Services/FileService.hpp"

#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultySerializedMethods.hpp>

#include <cmath>
#include <utility>

namespace SnoreSaber::Data
{
    namespace
    {
        double RoundAccuracy(double value)
        {
            return std::round(value * 100.0) / 100.0;
        }

        std::string GetReplayFileName(LeaderboardInfoMap& parent, const Score& score)
        {
            std::string playerId = score.leaderboardPlayerInfo.id.value_or("");
            if (!parent.beatmapLevel || playerId.empty())
            {
                return "";
            }

            std::string levelHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::GetSongHash(parent.beatmapKey);
            std::string characteristic = parent.beatmapKey.beatmapCharacteristic->serializedName;
            std::string songName = parent.beatmapLevel->songName;
            std::string difficultyName = GlobalNamespace::BeatmapDifficultySerializedMethods::SerializedName(parent.beatmapKey.difficulty);
            return SnoreSaber::Services::FileService::GetReplayFileName(levelHash, difficultyName, characteristic, playerId, songName);
        }
    }

    ScoreMap::ScoreMap(Score score,
                       LeaderboardInfoMap parent,
                       int maxScore,
                       SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService)
        : score(std::move(score)),
          parent(std::move(parent))
    {
        modifierText = this->score.modifiers;
        accuracy = maxScore > 0 ? RoundAccuracy((static_cast<double>(this->score.baseScore) / static_cast<double>(maxScore)) * 100.0) : 0.0;
        replayFileName = GetReplayFileName(this->parent, this->score);
        hasLocalReplay = replayStorageService && replayStorageService->LocalReplayExists(this->parent.beatmapLevel, this->parent.beatmapKey, this->score, replayFileName);
    }

    bool ScoreMap::HasReplay() const
    {
        return score.hasReplay || hasLocalReplay;
    }
}
