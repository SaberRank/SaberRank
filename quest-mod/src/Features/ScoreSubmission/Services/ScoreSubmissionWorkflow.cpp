#include "Features/ScoreSubmission/Services/ScoreSubmissionWorkflow.hpp"

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"
#include "Features/Leaderboards/Services/MaxScoreCache.hpp"
#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include "Services/ReplayService.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/GCUtil.hpp"
#include "logging.hpp"
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <optional>
#include <thread>

using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::Features::ScoreSubmission::Services, ScoreSubmissionWorkflow);

namespace SnoreSaber::Features::ScoreSubmission::Services
{
    namespace
    {
        constexpr int MaxUploadAttempts = 3;
        constexpr std::chrono::minutes StaleGameSessionAge{30};

        std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
            return value;
        }

        bool ContainsMessage(const ScoreUploadResult& result, const std::string& value)
        {
            std::string message = result.error.has_value() && !result.error->message.empty()
                ? result.error->message
                : result.message;
            return ToLower(message).find(value) != std::string::npos;
        }

        bool IsUploadTrustError(const ScoreUploadResult& result)
        {
            return ContainsMessage(result, "upload protocol") || ContainsMessage(result, "upload trust");
        }

        bool IsNonceError(const ScoreUploadResult& result)
        {
            return ContainsMessage(result, "nonce");
        }

        bool IsAuthenticationError(const ScoreUploadResult& result)
        {
            long statusCode = result.error.has_value() ? result.error->statusCode : 0;
            return statusCode == 401 || statusCode == 403 ||
                   ContainsMessage(result, "unauthorized") ||
                   ContainsMessage(result, "forbidden") ||
                   ContainsMessage(result, "authentication") ||
                   ContainsMessage(result, "session");
        }

        bool IsRetryable(const ScoreUploadResult& result)
        {
            long statusCode = result.error.has_value() ? result.error->statusCode : 0;
            return statusCode == 0 || statusCode >= 500 || IsUploadTrustError(result) || IsNonceError(result) || IsAuthenticationError(result);
        }

        float GetPlayOutcomeTime(LevelCompletionResults* levelCompletionResults, float playOutcomeTime, float recordedFailTime)
        {
            if (levelCompletionResults->levelEndStateType == LevelCompletionResults::LevelEndStateType::Failed && recordedFailTime > 0.0f)
            {
                return recordedFailTime;
            }

            return playOutcomeTime;
        }

        bool ShouldShowUploadState(Data::InternalLeaderboard const& internalLeaderboard, int multipliedScore, bool visibleUpload)
        {
            if (!visibleUpload)
            {
                return false;
            }

            if (!internalLeaderboard.leaderboard.has_value())
            {
                INFO("Failed to get leaderboard player score before upload");
                return true;
            }

            auto const& leaderboard = internalLeaderboard.leaderboard.value();
            if (!leaderboard.playerScore.has_value())
            {
                INFO("No existing player score, showing upload status");
                return true;
            }

            if (multipliedScore > leaderboard.playerScore.value().modifiedScore)
            {
                INFO("Score beats existing player score, showing upload status");
                return true;
            }

            INFO("Score did not beat existing player score, uploading silently");
            return false;
        }

        void Report(ScoreSubmissionWorkflow::StatusChanged statusChanged, ScoreUploadStatus status, std::string message)
        {
            if (statusChanged)
            {
                SnoreSaber::Utils::Async::Main([statusChanged, status, message = std::move(message)] {
                    statusChanged(ScoreSubmissionStatus::Progress(status, message));
                });
            }
        }

        void Complete(ScoreSubmissionWorkflow::Completed completed, ScoreUploadResult result, bool statusVisible)
        {
            if (completed)
            {
                SnoreSaber::Utils::Async::Main([completed = std::move(completed), result = std::move(result), statusVisible] {
                    completed(result, statusVisible);
                });
            }
        }

        ScoreUploadResult Error(std::string message)
        {
            return ScoreUploadResult::Failure(std::move(message));
        }

        ScoreUploadResult UploadWithRetries(Core::Api::SnoreSaberApiClient& apiClient,
                                            SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                                            std::optional<SnoreSaber::Data::GameSession>& session,
                                            SnoreSaber::Features::ScoreSubmission::Services::ScoreUploadPayloadBuilder* payloadBuilder,
                                            ScoreUploadPayload& payload,
                                            const std::vector<char>& replay,
                                            ScoreSubmissionWorkflow::StatusChanged statusChanged)
        {
            ScoreUploadResult uploadResult = Error("Failed to upload score");
            for (int attempt = 1; attempt <= MaxUploadAttempts; attempt++)
            {
                Report(statusChanged, ScoreUploadStatus::Uploading, "Uploading snore...");
                INFO("Uploading snore...");
                uploadResult = apiClient.UploadScore(session.value(), payload.encryptedScoreData, payload.uploadVersionHash, replay);
                if (uploadResult.success)
                {
                    INFO("Snore uploaded successfully");
                    return uploadResult;
                }

                ERROR("Failed to upload score: {:s}", uploadResult.message.c_str());
                if (uploadResult.error.has_value())
                {
                    INFO("Server response:\nHTTP code {:d}\nContent: {:s}", uploadResult.error->statusCode, uploadResult.error->rawBody.c_str());
                }

                if (attempt < MaxUploadAttempts && IsRetryable(uploadResult))
                {
                    bool shouldRefreshSession = IsUploadTrustError(uploadResult) || IsAuthenticationError(uploadResult);
                    if (shouldRefreshSession && !gameSessionService->RefreshGameSession())
                    {
                        return uploadResult;
                    }

                    if (shouldRefreshSession)
                    {
                        session = gameSessionService->GetGameSession();
                        if (!session.has_value())
                        {
                            return uploadResult;
                        }
                        payload = payloadBuilder->RebuildWithCurrentSession(payload);
                    }

                    Report(statusChanged, ScoreUploadStatus::Retrying, fmt::format("Failed, attempting again ({:d} of {:d} tries...)", attempt, MaxUploadAttempts));
                    ERROR("Score failed to upload, retrying");
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            }

            return uploadResult;
        }

        void UploadScore(SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService,
                         SnoreSaber::Utils::MaxScoreCache* maxScoreCache,
                         SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService* leaderboardQueryService,
                         SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                         SnoreSaber::Features::ScoreSubmission::Services::ScoreUploadPayloadBuilder* payloadBuilder,
                         BeatmapLevel* beatmapLevel,
                         BeatmapKey beatmapKey,
                         ScoreUploadPayload payload,
                         std::vector<char> replay,
                         bool saveLocalReplay,
                         ScoreSubmissionWorkflow::StatusChanged statusChanged,
                         ScoreSubmissionWorkflow::Completed completed)
        {
            SafePtr<SnoreSaber::ReplaySystem::ReplayStorageService> replayStorageServiceSafe(replayStorageService);
            SafePtr<SnoreSaber::Utils::MaxScoreCache> maxScoreCacheSafe(maxScoreCache);
            SafePtr<SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService> leaderboardQueryServiceSafe(leaderboardQueryService);
            SafePtr<SnoreSaber::Features::Players::Services::GameSessionService> gameSessionServiceSafe(gameSessionService);
            SafePtr<SnoreSaber::Features::ScoreSubmission::Services::ScoreUploadPayloadBuilder> payloadBuilderSafe(payloadBuilder);
            SafePtr<BeatmapLevel> beatmapLevelSafe(beatmapLevel);

            SnoreSaber::Utils::Async::Run([replayStorageServiceSafe, maxScoreCacheSafe, leaderboardQueryServiceSafe, gameSessionServiceSafe, payloadBuilderSafe, beatmapLevelSafe, beatmapKey, payload, replay = std::move(replay), saveLocalReplay, statusChanged, completed] {
                maxScoreCacheSafe->GetMaxScore(beatmapLevelSafe.ptr(), beatmapKey, gc_aware_function([replayStorageServiceSafe, leaderboardQueryServiceSafe, gameSessionServiceSafe, payloadBuilderSafe, beatmapLevelSafe, beatmapKey, payload, replay, saveLocalReplay, statusChanged, completed](int maxScore) {
                    leaderboardQueryServiceSafe->GetLeaderboardData(maxScore,
                        beatmapLevelSafe.ptr(), beatmapKey, PlatformLeaderboardsModel::ScoresScope::Global, 1, gc_aware_function([=](Data::InternalLeaderboard internalLeaderboard) {
                        bool showUploadState = ShouldShowUploadState(internalLeaderboard, payload.multipliedScore, saveLocalReplay);

                        if (payload.multipliedScore > maxScore)
                        {
                            INFO("Score was better than possible, not uploading!");
                            Complete(completed, Error("Failed to upload (score was impossible)"), showUploadState);
                            return;
                        }

                        ScoreUploadPayload uploadPayload = payload;
                        auto lastAuthenticatedAtUtc = gameSessionServiceSafe->LastAuthenticatedAtUtc();
                        if (lastAuthenticatedAtUtc != std::chrono::system_clock::time_point{} && std::chrono::system_clock::now() - lastAuthenticatedAtUtc > StaleGameSessionAge)
                        {
                            if (gameSessionServiceSafe->RefreshGameSession())
                            {
                                INFO("Refreshed stale SnoreSaber game session before upload.");
                                uploadPayload = payloadBuilderSafe->RebuildWithCurrentSession(uploadPayload);
                            }
                            else
                            {
                                WARN("Failed to refresh stale SnoreSaber game session before upload; trying cached session.");
                            }
                        }

                        std::optional<SnoreSaber::Data::GameSession> session = gameSessionServiceSafe->GetGameSession();
                        if (!session.has_value())
                        {
                            ERROR("SnoreSaber is not authenticated, score didn't upload");
                            Complete(completed, Error("SnoreSaber is not authenticated"), showUploadState);
                            return;
                        }

                        if (showUploadState)
                        {
                            Report(statusChanged, ScoreUploadStatus::Uploading, "Uploading snore...");
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
                        Core::Api::SnoreSaberApiClient apiClient;
                        ScoreUploadResult uploadResult = UploadWithRetries(apiClient, gameSessionServiceSafe.ptr(), session, payloadBuilderSafe.ptr(), uploadPayload, replay, showUploadState ? statusChanged : nullptr);

                        if (uploadResult.success && showUploadState)
                        {
                            replayStorageServiceSafe->SaveLocalReplay(gameSessionServiceSafe->GetLocalPlayerId(), beatmapKey, replay);
                        }

                        Complete(completed, uploadResult, showUploadState);
                    }),
                    false);
                }));
            });
        }
    }

    void ScoreSubmissionWorkflow::ctor(ScoreUploadPayloadBuilder* payloadBuilder,
                                       SnoreSaber::Utils::MaxScoreCache* maxScoreCache,
                                       SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService* leaderboardQueryService,
                                       SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService,
                                       SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
    {
        INVOKE_CTOR();
        _payloadBuilder = payloadBuilder;
        _maxScoreCache = maxScoreCache;
        _leaderboardQueryService = leaderboardQueryService;
        _replayStorageService = replayStorageService;
        _gameSessionService = gameSessionService;
    }

    void ScoreSubmissionWorkflow::SubmitScore(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LevelCompletionResults* levelCompletionResults, float playOutcomeTime, std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride, bool saveLocalReplay, StatusChanged statusChanged, Completed completed)
    {
        auto replayResult = SnoreSaber::Services::ReplayService::WriteSerializedReplay();
        if (!replayResult.has_value() || replayResult->replay.empty())
        {
            ERROR("Failed to serialize replay, not uploading score");
            Complete(completed, Error("Failed to upload (failed to serialize replay)"), saveLocalReplay);
            return;
        }

        float outcomeTime = GetPlayOutcomeTime(levelCompletionResults, playOutcomeTime, replayResult->failTime);
        ScoreUploadPayload payload = _payloadBuilder->Build(beatmapLevel, beatmapKey, levelCompletionResults, outcomeTime, playOutcomeOverride);
        INFO("Upload payload size: data={:d} chars, replay={:d} bytes", payload.encryptedScoreData.size(), replayResult->replay.size());
        UploadScore(_replayStorageService, _maxScoreCache, _leaderboardQueryService, _gameSessionService, _payloadBuilder, beatmapLevel, beatmapKey, payload, replayResult->replay, saveLocalReplay, statusChanged, completed);
    }

    void ScoreSubmissionWorkflow::DiscardReplay()
    {
        SnoreSaber::Services::ReplayService::DiscardReplay();
    }

    void ScoreSubmissionWorkflow::WriteReplayOnly(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LevelCompletionResults* levelCompletionResults, float playOutcomeTime)
    {
        (void)beatmapLevel;
        (void)beatmapKey;
        (void)levelCompletionResults;
        (void)playOutcomeTime;
        SnoreSaber::Services::ReplayService::DiscardReplay();
    }
}
