#include "Features/Leaderboards/Services/LeaderboardQueryService.hpp"

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Data/Private/Settings.hpp"
#include "Features/Leaderboards/Domain/LeaderboardQuery.hpp"
#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Features/Leaderboards/Services/LeaderboardService.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/GCUtil.hpp"
#include "Utils/SafePtr.hpp"
#include "logging.hpp"

#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultyMethods.hpp>

#include <algorithm>
#include <cctype>

using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::Features::Leaderboards::Services, LeaderboardQueryService);

namespace SnoreSaber::Features::Leaderboards::Services
{
    namespace
    {
        SnoreSaber::Data::LeaderboardQueryScope QueryScopeFor(PlatformLeaderboardsModel::ScoresScope scope)
        {
            switch (scope)
            {
                case PlatformLeaderboardsModel::ScoresScope::AroundPlayer:
                    return SnoreSaber::Data::LeaderboardQueryScope::AroundPlayer;
                case PlatformLeaderboardsModel::ScoresScope::Friends:
                    return SnoreSaber::Data::LeaderboardQueryScope::Friends;
                default:
                    return SnoreSaber::Data::LeaderboardQueryScope::Global;
            }
        }

        SnoreSaber::Data::LeaderboardQueryScope GetLocationScope()
        {
            std::string mode = SnoreSaber::Data::Private::Settings::locationFilterMode;
            std::transform(mode.begin(), mode.end(), mode.begin(), [](unsigned char c) { return std::tolower(c); });

            if (mode == "region")
                return SnoreSaber::Data::LeaderboardQueryScope::Region;

            if (mode != "country")
                ERROR("Invalid location filter mode, falling back to country");

            return SnoreSaber::Data::LeaderboardQueryScope::Country;
        }

        SnoreSaber::Data::LeaderboardQuery GetLeaderboardQuery(BeatmapKey beatmapKey, PlatformLeaderboardsModel::ScoresScope scope, int page, bool filterAroundCountry)
        {
            SnoreSaber::Data::LeaderboardQuery query;
            query.songHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::GetSongHash(beatmapKey);
            query.gameMode = "Solo" + std::string(beatmapKey.beatmapCharacteristic->serializedName);
            query.difficulty = BeatmapDifficultyMethods::DefaultRating(beatmapKey.difficulty);
            query.page = page;
            query.limit = 10;
            query.scope = filterAroundCountry ? GetLocationScope() : QueryScopeFor(scope);
            query.hideNoArrows = SnoreSaber::Data::Private::Settings::hideNAScoresFromLeaderboard;
            return query;
        }

        bool IsLeaderboardNotFoundResponse(const SnoreSaber::Core::Api::Generated::ApiException& exception)
        {
            if (exception.statusCode != 404)
                return false;

            std::string errorText = exception.response + " " + exception.message;
            std::transform(errorText.begin(), errorText.end(), errorText.begin(), [](unsigned char c) { return std::tolower(c); });
            return errorText.find("leaderboard not found") != std::string::npos;
        }

        bool IsAuthenticationResponse(const SnoreSaber::Core::Api::Generated::ApiException& exception)
        {
            if (exception.statusCode == 401 || exception.statusCode == 403)
                return true;

            std::string errorText = exception.response + " " + exception.message;
            std::transform(errorText.begin(), errorText.end(), errorText.begin(), [](unsigned char c) { return std::tolower(c); });
            return errorText.find("unauthorized") != std::string::npos ||
                   errorText.find("forbidden") != std::string::npos ||
                   errorText.find("authentication") != std::string::npos ||
                   errorText.find("session") != std::string::npos;
        }

        std::string GetPlayerId(const std::optional<SnoreSaber::Data::GameSession>& session, SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
        {
            if (session.has_value())
            {
                return session->playerId;
            }

            return gameSessionService->GetLocalPlayerId();
        }

        SnoreSaber::Data::LeaderboardSnapshot GetLeaderboardWithSessionRefresh(SnoreSaber::Core::Api::SnoreSaberApiClient& apiClient,
                                                                               SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                                                                               const SnoreSaber::Data::LeaderboardQuery& query,
                                                                               std::optional<SnoreSaber::Data::GameSession>& session)
        {
            try
            {
                return apiClient.GetLeaderboard(query, session.has_value() ? &session.value() : nullptr);
            }
            catch (const SnoreSaber::Core::Api::Generated::ApiException& exception)
            {
                if (!session.has_value() || !IsAuthenticationResponse(exception) || !gameSessionService->RefreshGameSession())
                {
                    throw;
                }

                WARN("SnoreSaber game session expired while loading leaderboard; refreshed session and retrying.");
                session = gameSessionService->GetGameSession();
                return apiClient.GetLeaderboard(query, session.has_value() ? &session.value() : nullptr);
            }
        }
    }

    void LeaderboardQueryService::ctor(LeaderboardPlayerScoreCache* playerScoreCache,
                                       SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService,
                                       SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
    {
        INVOKE_CTOR();
        _playerScoreCache = playerScoreCache;
        _replayStorageService = replayStorageService;
        _gameSessionService = gameSessionService;
    }

    void LeaderboardQueryService::GetLeaderboardData(int maxMultipliedScore,
                                                     BeatmapLevel* beatmapLevel,
                                                     BeatmapKey beatmapKey,
                                                     PlatformLeaderboardsModel::ScoresScope scope,
                                                     int page,
                                                     std::function<void(SnoreSaber::Data::InternalLeaderboard)> finished,
                                                     bool filterAroundCountry)
    {
        if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::TryGetSongHash(beatmapKey).has_value())
        {
            finished(SnoreSaber::Services::LeaderboardService::GetLeaderboardError(SnoreSaber::Utils::SnoreSaberBeatmapKey::IsWip(beatmapKey) ? "SnoreSaber doesn't support WIP levels" : "SnoreSaber doesn't support this level"));
            return;
        }

        SafePtr<LeaderboardQueryService> self(this);
        SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);

        SnoreSaber::Utils::Async::Run([self, beatmapLevelSafe, beatmapKey, scope, page, filterAroundCountry, maxMultipliedScore, finished] {
            try
            {
                SnoreSaber::Core::Api::SnoreSaberApiClient apiClient;
                SnoreSaber::Data::LeaderboardQuery query = GetLeaderboardQuery(beatmapKey, scope, page, filterAroundCountry);
                std::optional<SnoreSaber::Data::GameSession> session = self->_gameSessionService->GetGameSession();
                SnoreSaber::Data::LeaderboardSnapshot leaderboard = GetLeaderboardWithSessionRefresh(apiClient, self->_gameSessionService, query, session);
                self->_playerScoreCache->Remember(query, GetPlayerId(session, self->_gameSessionService), leaderboard.playerScore);
                finished(SnoreSaber::Services::LeaderboardService::ParseLeaderboardData(std::move(leaderboard), beatmapLevelSafe.ptr(), beatmapKey, scope, page, filterAroundCountry, maxMultipliedScore, self->_replayStorageService));
            }
            catch (const SnoreSaber::Core::Api::Generated::ApiException& exception)
            {
                if (IsLeaderboardNotFoundResponse(exception))
                    finished(SnoreSaber::Services::LeaderboardService::GetLeaderboardError("Play this level to create a SnoreSaber leaderboard", true));
                else if (exception.statusCode == 404)
                    finished(SnoreSaber::Services::LeaderboardService::GetLeaderboardError("No scores on this leaderboard!"));
                else
                    finished(SnoreSaber::Services::LeaderboardService::GetLeaderboardError("Received invalid response from server"));
            }
            catch (const std::exception& exception)
            {
                ERROR("Failed to load SnoreSaber leaderboard: {:s}", exception.what());
                finished(SnoreSaber::Services::LeaderboardService::GetLeaderboardError("Received invalid response from server"));
            }
        });
    }
}
