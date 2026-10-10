#include "Features/Leaderboards/Services/LeaderboardPlayerScoreCache.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"

#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultyMethods.hpp>

using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::Features::Leaderboards::Services, LeaderboardPlayerScoreCache);

namespace SnoreSaber::Features::Leaderboards::Services
{
    namespace
    {
        std::string CreateKey(const std::string& songHash, const std::string& gameMode, int difficulty, const std::optional<int>& realmId, const std::string& playerId)
        {
            std::string realm = realmId.has_value() ? std::to_string(realmId.value()) : "";
            return playerId + "|" + realm + "|" + songHash + "|" + gameMode + "|" + std::to_string(difficulty);
        }
    }

    void LeaderboardPlayerScoreCache::Remember(const SnoreSaber::Data::LeaderboardQuery& query, const std::string& playerId, const std::optional<SnoreSaber::Data::Score>& score)
    {
        if (playerId.empty())
        {
            return;
        }

        std::string key = CreateKey(query.songHash, query.gameMode, query.difficulty, query.realmId, playerId);
        if (!score.has_value())
        {
            _scores.erase(key);
            return;
        }

        _scores[key] = score.value();
    }

    bool LeaderboardPlayerScoreCache::TryGet(BeatmapKey beatmapKey, const std::string& playerId, SnoreSaber::Data::Score& score) const
    {
        std::optional<std::string> songHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::TryGetSongHash(beatmapKey);
        if (playerId.empty() || !songHash.has_value())
        {
            return false;
        }

        std::string key = CreateKey(songHash.value(),
                                    "Solo" + std::string(beatmapKey.beatmapCharacteristic->serializedName),
                                    BeatmapDifficultyMethods::DefaultRating(beatmapKey.difficulty),
                                    std::nullopt,
                                    playerId);
        auto itr = _scores.find(key);
        if (itr == _scores.end())
        {
            return false;
        }

        score = itr->second;
        return true;
    }
}
