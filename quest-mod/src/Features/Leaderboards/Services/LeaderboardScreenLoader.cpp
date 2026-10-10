#include "Features/Leaderboards/Services/LeaderboardScreenLoader.hpp"

#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"
#include "Features/Leaderboards/Domain/LeaderboardDetails.hpp"
#include "Features/Leaderboards/Domain/Score.hpp"
#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Utils/GCUtil.hpp"

#include <utility>
#include <fmt/format.h>

using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::Features::Leaderboards::Services, LeaderboardScreenLoader);

namespace SnoreSaber::Features::Leaderboards::Services
{
    namespace
    {
        std::string GetRankedStatusText(const SnoreSaber::Data::LeaderboardDetails& leaderboardInfo)
        {
            if (leaderboardInfo.status == SnoreSaber::Data::LeaderboardStatus::Ranked)
            {
                return leaderboardInfo.positiveModifiers ? fmt::format("Ranked (DA = +0.02, GN +0.04)\nStars: {:.2f}", leaderboardInfo.stars) : fmt::format("Ranked (modifiers disabled)\nStars: {:.2f}", leaderboardInfo.stars);
            }

            if (leaderboardInfo.status == SnoreSaber::Data::LeaderboardStatus::Qualified)
            {
                return "Qualified";
            }

            if (leaderboardInfo.status == SnoreSaber::Data::LeaderboardStatus::Loved)
            {
                return "Loved";
            }

            return "Unranked";
        }

        int GetPlayerScoreIndex(const std::vector<SnoreSaber::Data::Score>& scores, const std::string& playerId)
        {
            for (int i = 0; i < scores.size(); i++)
            {
                if (scores[i].leaderboardPlayerInfo.id == playerId)
                {
                    return i;
                }
            }
            return -1;
        }

        bool CanPageScopeValue(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry)
        {
            return scope != PlatformLeaderboardsModel::ScoresScope::AroundPlayer || filterAroundCountry;
        }

        SnoreSaber::Data::LeaderboardScreenState CreateLoadedState(SnoreSaber::Data::InternalLeaderboard internalLeaderboard, PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry, int page, const std::string& playerId)
        {
            bool canPage = CanPageScopeValue(scope, filterAroundCountry);
            if (internalLeaderboard.leaderboardNotFound)
            {
                return SnoreSaber::Data::LeaderboardScreenState::Failed(SnoreSaber::Data::LeaderboardScreenStatus::NoLeaderboard, "Play this level to create a SnoreSaber leaderboard", true,
                                                                        std::nullopt, "Unranked", false, page);
            }

            if (!internalLeaderboard.leaderboard.has_value())
            {
                std::string errorText = "No scores on this leaderboard, be the first! 0x1";
                if (internalLeaderboard.leaderboardItems->Count > 0 && internalLeaderboard.leaderboardItems->get_Item(0))
                {
                    errorText = std::string(internalLeaderboard.leaderboardItems->get_Item(0)->playerName);
                }

                return SnoreSaber::Data::LeaderboardScreenState::Failed(SnoreSaber::Data::LeaderboardScreenStatus::Error, errorText, false, std::move(internalLeaderboard), "", canPage, page);
            }

            int playerScoreIndex = GetPlayerScoreIndex(internalLeaderboard.leaderboard->scores.items, playerId);
            std::string rankedStatus = internalLeaderboard.leaderboardMap.has_value()
                ? GetRankedStatusText(internalLeaderboard.leaderboardMap->leaderboardInfo.leaderboard)
                : "";
            if (internalLeaderboard.leaderboardItems->Count != 0)
            {
                if (scope == PlatformLeaderboardsModel::ScoresScope::AroundPlayer && playerScoreIndex == -1 && !filterAroundCountry)
                {
                    return SnoreSaber::Data::LeaderboardScreenState::Failed(SnoreSaber::Data::LeaderboardScreenStatus::NoPlayerScore, "You haven't set a snore on this leaderboard", true,
                                                                            std::move(internalLeaderboard), rankedStatus, canPage, page);
                }

                return SnoreSaber::Data::LeaderboardScreenState::Loaded(std::move(internalLeaderboard), playerScoreIndex, rankedStatus, canPage, page);
            }

            std::string emptyText = page > 1 ? "No scores on this page" : "No scores on this leaderboard, be the first!";
            return SnoreSaber::Data::LeaderboardScreenState::Failed(SnoreSaber::Data::LeaderboardScreenStatus::Empty, emptyText, true, std::move(internalLeaderboard), rankedStatus, canPage, page);
        }

        LoadResult Result(SnoreSaber::Data::LeaderboardScreenState state, int maxScore = 0)
        {
            LoadResult result;
            result.state = std::move(state);
            result.maxScore = maxScore;
            return result;
        }
    }

    void LeaderboardScreenLoader::ctor(SnoreSaber::Utils::MaxScoreCache* maxScoreCache,
                                       LeaderboardQueryService* leaderboardQueryService,
                                       SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
    {
        INVOKE_CTOR();
        _maxScoreCache = maxScoreCache;
        _leaderboardQueryService = leaderboardQueryService;
        _gameSessionService = gameSessionService;
    }

    bool LeaderboardScreenLoader::CanPageScope(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry) const
    {
        return CanPageScopeValue(scope, filterAroundCountry);
    }

    void LeaderboardScreenLoader::Load(BeatmapLevel* beatmapLevel,
              BeatmapKey beatmapKey,
              PlatformLeaderboardsModel::ScoresScope scope,
              int page,
              bool filterAroundCountry,
              std::function<void(LoadResult)> finished)
    {
        if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(beatmapKey))
        {
            std::string errorText = SnoreSaber::Utils::SnoreSaberBeatmapKey::IsWip(beatmapKey) ? "SnoreSaber doesn't support WIP levels" : "SnoreSaber doesn't support this level";
            finished(Result(SnoreSaber::Data::LeaderboardScreenState::Failed(SnoreSaber::Data::LeaderboardScreenStatus::Error, errorText, false, std::nullopt, "", false, page)));
            return;
        }

        if (_gameSessionService->GetStatus() == SnoreSaber::Services::PlayerService::LoginStatus::Error)
        {
            finished(Result(SnoreSaber::Data::LeaderboardScreenState::Failed(SnoreSaber::Data::LeaderboardScreenStatus::Error, "SnoreSaber authentication failed, please restart Beat Saber", false, std::nullopt,
                                                                             "", false, page)));
            return;
        }

        if (_gameSessionService->GetStatus() != SnoreSaber::Services::PlayerService::LoginStatus::Success)
        {
            _gameSessionService->EnsureAuthenticated(false, [](SnoreSaber::Services::PlayerService::LoginStatus) {});
            finished(Result(SnoreSaber::Data::LeaderboardScreenState::Loading(page, CanPageScope(scope, filterAroundCountry))));
            return;
        }

        SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);
        SafePtr<LeaderboardQueryService> leaderboardQueryServiceSafe(_leaderboardQueryService);
        std::string playerId = _gameSessionService->GetLocalPlayerId();
        _maxScoreCache->GetMaxScore(beatmapLevelSafe.ptr(), beatmapKey, gc_aware_function([leaderboardQueryServiceSafe, beatmapLevelSafe, beatmapKey, scope, page, filterAroundCountry, finished, playerId](int maxScore) {
            leaderboardQueryServiceSafe->GetLeaderboardData(maxScore,
                                                   beatmapLevelSafe.ptr(),
                                                   beatmapKey,
                                                   scope,
                                                   page,
                                                   gc_aware_function([=](SnoreSaber::Data::InternalLeaderboard internalLeaderboard) {
                                                       finished(Result(CreateLoadedState(std::move(internalLeaderboard), scope, filterAroundCountry, page, playerId), maxScore));
                                                   }),
                                                   filterAroundCountry);
        }));
    }
}
