#include "Utils/StringUtils.hpp"

#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"

#include <GlobalNamespace/BeatmapDifficulty.hpp>
#include <GlobalNamespace/GameplayModifierParamsSO.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include "Features/Leaderboards/Services/LeaderboardService.hpp"

#include <UnityEngine/Networking/UnityWebRequest.hpp>
#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include "logging.hpp"
#include "static.hpp"
#include <paper2_scotland2/shared/string_convert.hpp>

using namespace GlobalNamespace;
using namespace StringUtils;
using namespace SnoreSaber;

namespace SnoreSaber::Services::LeaderboardService
{
    Data::InternalLeaderboard GetLeaderboardError(std::string error, bool leaderboardNotFound)
    {
        auto scores = ListW<LeaderboardTableView::ScoreData*>::New();
        scores->Add(LeaderboardTableView::ScoreData::New_ctor(0, error, 0, false));
        auto internalLeaderboard = Data::InternalLeaderboard(scores, {});
        internalLeaderboard.leaderboardNotFound = leaderboardNotFound;
        return internalLeaderboard;
    }

    Data::InternalLeaderboard ParseLeaderboardData(Data::LeaderboardSnapshot currentLeaderboard, BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, PlatformLeaderboardsModel::ScoresScope scope,
                                                   int page, bool filterAroundCountry, int maxScore, SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService)
    {
        auto scores = ListW<LeaderboardTableView::ScoreData*>::New();
        auto modifiers = ListW<GameplayModifierParamsSO*>::New();
        std::vector<std::string> profilePictures;
        Data::LeaderboardMap leaderboardMap(currentLeaderboard, beatmapLevel, beatmapKey, maxScore, replayStorageService);

        for (auto& score : currentLeaderboard.scores.items)
        {
            auto& leaderboardPlayerInfo = score.leaderboardPlayerInfo;
            std::u16string formattedScore = FormatScore(((double)score.baseScore / (double)maxScore) * 100.0);
            std::u16string formattedPP = FormatPP(score);
            std::u16string result = Resize(leaderboardPlayerInfo.name.value_or(u"unknown") + formattedScore + formattedPP, 75);
            INFO("score: {:d}, rank: {:d}", score.modifiedScore, score.rank);
            scores->Add(LeaderboardTableView::ScoreData::New_ctor(score.modifiedScore, result, score.rank, false));
            profilePictures.push_back(leaderboardPlayerInfo.profilePicture);
        }

        if (scores.size() == 0)
        {
            if (scope == PlatformLeaderboardsModel::ScoresScope::AroundPlayer && !filterAroundCountry)
                scores->Add(LeaderboardTableView::ScoreData::New_ctor(0, "You haven't set a score on this leaderboard", 0, false));

            return Data::InternalLeaderboard(scores, profilePictures, std::make_optional<Data::LeaderboardSnapshot>(std::move(currentLeaderboard)), std::make_optional<Data::LeaderboardMap>(std::move(leaderboardMap)));
        }

        return Data::InternalLeaderboard(scores, profilePictures, std::make_optional<Data::LeaderboardSnapshot>(std::move(currentLeaderboard)), std::make_optional<Data::LeaderboardMap>(std::move(leaderboardMap)));
    }
} // namespace SnoreSaber::Services::LeaderboardService
