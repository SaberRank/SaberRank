#include "Features/ScoreSubmission/ScoreSubmissionController.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Features/Replays/ReplayStateRegistry.hpp"
#include "Utils/GCUtil.hpp"
#include "Utils/SafePtr.hpp"
#include "Utils/StringUtils.hpp"
#include "logging.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/MultiplayerLevelCompletionResults.hpp>
#include <GlobalNamespace/MultiplayerPlayerResultsData.hpp>
#include <GlobalNamespace/PracticeViewController.hpp>
#include <UnityEngine/Resources.hpp>
#include <metacore/shared/game.hpp>
#include <algorithm>
#include <cctype>
#include <optional>

using namespace GlobalNamespace;
using namespace SnoreSaber::Data::Private;

DEFINE_TYPE(SnoreSaber::Features::ScoreSubmission, ScoreSubmissionController);

namespace SnoreSaber::Features::ScoreSubmission
{
    namespace
    {
        enum class ScoreSubmissionAction
        {
            Ignore,
            WriteReplayOnly,
            SubmitScore,
        };

        enum class ScoreSubmissionVisibility
        {
            Visible,
            Silent,
        };

        struct ScoreSubmissionRequest
        {
            StringW gameMode;
            BeatmapLevel* beatmapLevel;
            BeatmapKey beatmapKey;
            LevelCompletionResults* results;
            bool practicing;
            float playOutcomeTime;
        };

        struct ScoreSubmissionDecision
        {
            ScoreSubmissionAction action;
            ScoreSubmissionVisibility visibility;
            std::string reason;

            static ScoreSubmissionDecision Ignore(std::string reason)
            {
                return {ScoreSubmissionAction::Ignore, ScoreSubmissionVisibility::Silent, std::move(reason)};
            }

            static ScoreSubmissionDecision WriteReplayOnly(std::string reason)
            {
                return {ScoreSubmissionAction::WriteReplayOnly, ScoreSubmissionVisibility::Silent, std::move(reason)};
            }

            static ScoreSubmissionDecision SubmitScore(ScoreSubmissionVisibility visibility, std::string reason)
            {
                return {ScoreSubmissionAction::SubmitScore, visibility, std::move(reason)};
            }
        };

        std::string ActionText(ScoreSubmissionAction action)
        {
            switch (action)
            {
                case ScoreSubmissionAction::Ignore:
                    return "ignore";
                case ScoreSubmissionAction::WriteReplayOnly:
                    return "write replay only";
                case ScoreSubmissionAction::SubmitScore:
                    return "submit score";
            }

            return "unknown";
        }

        bool IsSupportedGameMode(StringW gameMode)
        {
            return gameMode == "Solo" || gameMode == "Multiplayer";
        }

        bool IsPracticeViewActive()
        {
            auto practiceViewController = UnityEngine::Resources::FindObjectsOfTypeAll<PracticeViewController*>()->FirstOrDefault();
            return practiceViewController && practiceViewController->isInViewControllerHierarchy;
        }

        float GetCurrentSongTime()
        {
            auto audioTimeSyncController = UnityEngine::Resources::FindObjectsOfTypeAll<AudioTimeSyncController*>()->FirstOrDefault();
            return audioTimeSyncController ? audioTimeSyncController->songTime : 0.0f;
        }

        std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
            return value;
        }

        std::string GetPlayerId(SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService)
        {
            std::optional<SnoreSaber::Data::GameSession> session = gameSessionService->GetGameSession();
            return session.has_value() ? session->playerId : gameSessionService->GetLocalPlayerId();
        }

        bool ShouldShowUploadStatus(SnoreSaber::Features::Leaderboards::Services::LeaderboardPlayerScoreCache* playerScoreCache,
                                    SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                                    ScoreSubmissionRequest const& request)
        {
            SnoreSaber::Data::Score playerScore;
            if (!playerScoreCache->TryGet(request.beatmapKey, GetPlayerId(gameSessionService), playerScore))
            {
                INFO("No cached API player score for this leaderboard; showing score upload status");
                return true;
            }

            if (!playerScore.personalBest)
            {
                INFO("Cached API player score is not marked as a PB; showing score upload status");
                return true;
            }

            if (ToLower(playerScore.playOutcome) != "clear")
            {
                INFO("Cached API player score is not a clear; showing score upload status");
                return true;
            }

            if (request.results->multipliedScore > playerScore.modifiedScore)
            {
                INFO("Score beats cached API PB, showing score upload status");
                return true;
            }

            INFO("Score did not beat cached API PB, uploading silently");
            return false;
        }

        std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> GetPlayOutcomeOverride(SnoreSaber::Features::Live::Compete::Services::CompeteGameplayState* competeGameplayState,
                                                                                               ScoreSubmissionRequest const& request)
        {
            std::string songHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::TryGetSongHash(request.beatmapKey).value_or("");
            if (competeGameplayState->TryConsumeHostStop(songHash))
            {
                return SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome::Quit;
            }

            return std::nullopt;
        }

        ScoreSubmissionDecision Decide(ScoreSubmissionRequest const& request, std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride)
        {
            if (StringUtils::GetEnv("disable_ss_upload") == "1")
            {
                return ScoreSubmissionDecision::Ignore("disabled by disable_ss_upload");
            }

            if (SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled())
            {
                return ScoreSubmissionDecision::Ignore("replay playback is active");
            }

            if (!IsSupportedGameMode(request.gameMode))
            {
                return ScoreSubmissionDecision::Ignore("unsupported game mode");
            }

            if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(request.beatmapKey))
            {
                return ScoreSubmissionDecision::Ignore("unsupported or WIP SnoreSaber level id");
            }

            if (MetaCore::Game::IsScoreSubmissionDisabled())
            {
                return ScoreSubmissionDecision::Ignore("score submission is disabled");
            }

            if (request.practicing || IsPracticeViewActive())
            {
                return ScoreSubmissionDecision::WriteReplayOnly("practice run");
            }

            if (request.results->multipliedScore <= 0)
            {
                return ScoreSubmissionDecision::Ignore("score is 0, server would reject it");
            }

            if (playOutcomeOverride.has_value() && *playOutcomeOverride == SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome::Quit)
            {
                return ScoreSubmissionDecision::SubmitScore(ScoreSubmissionVisibility::Silent, "live map was stopped by host");
            }

            if (request.results->levelEndStateType == LevelCompletionResults::LevelEndStateType::Failed)
            {
                return ScoreSubmissionDecision::SubmitScore(ScoreSubmissionVisibility::Silent, "level failed");
            }

            if (request.results->levelEndAction == LevelCompletionResults::LevelEndAction::Restart)
            {
                return ScoreSubmissionDecision::SubmitScore(ScoreSubmissionVisibility::Silent, "level was restarted");
            }

            if (request.results->levelEndAction == LevelCompletionResults::LevelEndAction::Quit)
            {
                return ScoreSubmissionDecision::SubmitScore(ScoreSubmissionVisibility::Silent, "level was quit");
            }

            if (request.results->levelEndStateType != LevelCompletionResults::LevelEndStateType::Cleared)
            {
                return ScoreSubmissionDecision::WriteReplayOnly(fmt::format("level was not cleared ({:d}, {:d})", static_cast<int>(request.results->levelEndStateType), static_cast<int>(request.results->levelEndAction)));
            }

            return ScoreSubmissionDecision::SubmitScore(ScoreSubmissionVisibility::Visible, "level cleared");
        }

    }

    void ScoreSubmissionController::ctor(Services::ScoreSubmissionWorkflow* submissionWorkflow,
                                         Services::ScoreSubmissionService* scoreSubmissionService,
                                         SnoreSaber::Features::Leaderboards::LeaderboardStatusController* leaderboardStatusController,
                                         SnoreSaber::Features::Leaderboards::Services::LeaderboardPlayerScoreCache* playerScoreCache,
                                         SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                                         SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService,
                                         SnoreSaber::Features::Live::Compete::Services::CompeteGameplayState* competeGameplayState)
    {
        INVOKE_CTOR();
        _submissionWorkflow = submissionWorkflow;
        _scoreSubmissionService = scoreSubmissionService;
        _leaderboardStatusController = leaderboardStatusController;
        _playerScoreCache = playerScoreCache;
        _gameSessionService = gameSessionService;
        _liveReplayStreamingService = liveReplayStreamingService;
        _competeGameplayState = competeGameplayState;
        _isUploading = false;
        _visibleUploadCount = 0;
        INFO("Score submission controller setup!");
    }

    void ScoreSubmissionController::Initialize()
    {
        SafePtr<ScoreSubmissionController> self(this);
        _scoreSubmissionService->RegisterCallbacks(
            [self](StandardLevelScenesTransitionSetupDataSO* transition, LevelCompletionResults* levelCompletionResults) {
                self->HandleStandardLevelFinished(transition, levelCompletionResults);
            },
            [self](MultiplayerLevelScenesTransitionSetupDataSO* transition, MultiplayerResultsData* multiplayerResultsData) {
                self->HandleMultiplayerLevelFinished(transition, multiplayerResultsData);
            });
    }

    void ScoreSubmissionController::Dispose()
    {
        INFO("Score submission controller disposed");
        _scoreSubmissionService->ClearCallbacks();
    }

    bool ScoreSubmissionController::IsUploading() const
    {
        return _isUploading;
    }

    void ScoreSubmissionController::HandleStandardLevelFinished(StandardLevelScenesTransitionSetupDataSO* transition, LevelCompletionResults* levelCompletionResults)
    {
        if (!transition || !levelCompletionResults)
        {
            return;
        }

        HandleLevelFinished(transition->gameMode, transition->beatmapLevel, transition->beatmapKey, levelCompletionResults, transition->practiceSettings, GetCurrentSongTime());
    }

    void ScoreSubmissionController::HandleLevelFinished(StringW gameMode,
                                                        BeatmapLevel* beatmapLevel,
                                                        BeatmapKey beatmapKey,
                                                        LevelCompletionResults* levelCompletionResults,
                                                        bool practicing,
                                                        float playOutcomeTime)
    {
        if (!beatmapLevel || !beatmapKey.beatmapCharacteristic || !levelCompletionResults)
        {
            ERROR("Ignoring score submission with missing beatmap data");
            return;
        }

        ScoreSubmissionRequest request {gameMode, beatmapLevel, beatmapKey, levelCompletionResults, practicing, playOutcomeTime};
        std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride = GetPlayOutcomeOverride(_competeGameplayState, request);
        _liveReplayStreamingService->Complete(request.results, request.playOutcomeTime, playOutcomeOverride);
        ScoreSubmissionDecision decision = Decide(request, playOutcomeOverride);
        INFO("Score submission decision: {:s} ({:s})", ActionText(decision.action).c_str(), decision.reason.c_str());

        switch (decision.action)
        {
            case ScoreSubmissionAction::Ignore:
                _submissionWorkflow->DiscardReplay();
                return;
            case ScoreSubmissionAction::WriteReplayOnly:
                _submissionWorkflow->WriteReplayOnly(request.beatmapLevel, request.beatmapKey, request.results, request.playOutcomeTime);
                return;
            case ScoreSubmissionAction::SubmitScore: {
                bool visibleUpload = decision.visibility == ScoreSubmissionVisibility::Visible && ShouldShowUploadStatus(_playerScoreCache, _gameSessionService, request);
                SubmitScore(request.beatmapLevel, request.beatmapKey, request.results, request.playOutcomeTime, playOutcomeOverride, visibleUpload);
                return;
            }
        }
    }

    void ScoreSubmissionController::SubmitScore(BeatmapLevel* beatmapLevel, BeatmapKey beatmapKey, LevelCompletionResults* levelCompletionResults, float playOutcomeTime, std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride, bool visibleUpload)
    {
        if (visibleUpload)
        {
            _visibleUploadCount++;
            _isUploading = true;
        }

        SafePtr<ScoreSubmissionController> self(this);
        SafePtr<SnoreSaber::Features::Leaderboards::LeaderboardStatusController> leaderboardStatusControllerSafe(_leaderboardStatusController);
        _submissionWorkflow->SubmitScore(beatmapLevel, beatmapKey, levelCompletionResults, playOutcomeTime, playOutcomeOverride, visibleUpload,
            visibleUpload ? Services::ScoreSubmissionWorkflow::StatusChanged([leaderboardStatusControllerSafe](ScoreSubmissionStatus status) {
                leaderboardStatusControllerSafe->ApplySubmissionStatus(status);
            }) : nullptr,
            [self, leaderboardStatusControllerSafe, visibleUpload](ScoreUploadResult result, bool statusVisible) {
                if (result.success)
                {
                    INFO("Snore uploaded!");
                }
                else
                {
                    ERROR("Failed to upload score: {:s}", result.message.c_str());
                }

                if (statusVisible)
                {
                    leaderboardStatusControllerSafe->ApplySubmissionStatus(ScoreSubmissionStatus::FromResult(result));
                }

                if (!visibleUpload)
                {
                    return;
                }

                self->_visibleUploadCount--;
                if (self->_visibleUploadCount <= 0)
                {
                    self->_visibleUploadCount = 0;
                    self->_isUploading = false;
                }
            });
    }

    void ScoreSubmissionController::HandleMultiplayerLevelFinished(MultiplayerLevelScenesTransitionSetupDataSO* transition, MultiplayerResultsData* multiplayerResultsData)
    {
        if (!transition || !transition->beatmapLevel)
        {
            return;
        }
        if (!multiplayerResultsData || !multiplayerResultsData->localPlayerResultData || !multiplayerResultsData->localPlayerResultData->multiplayerLevelCompletionResults)
        {
            return;
        }

        MultiplayerLevelCompletionResults* multiplayerResults = multiplayerResultsData->localPlayerResultData->multiplayerLevelCompletionResults;
        if (!multiplayerResults->levelCompletionResults)
        {
            return;
        }
        if (multiplayerResults->playerLevelEndReason == MultiplayerLevelCompletionResults::MultiplayerPlayerLevelEndReason::HostEndedLevel)
        {
            return;
        }

        HandleLevelFinished(transition->gameMode, transition->beatmapLevel, transition->beatmapKey, multiplayerResults->levelCompletionResults, false, GetCurrentSongTime());
    }
}
