#pragma once

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp>
#include <GlobalNamespace/MultiplayerResultsData.hpp>
#include <GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp>

#include "Core/Gameplay/SnoreSaberPlayOutcome.hpp"
#include "Features/Leaderboards/LeaderboardStatusController.hpp"
#include "Features/Leaderboards/Services/LeaderboardPlayerScoreCache.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Replay/LiveReplayStreamingService.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionService.hpp"
#include "Features/ScoreSubmission/Services/ScoreSubmissionWorkflow.hpp"
#include <System/IDisposable.hpp>
#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <optional>

DECLARE_CLASS_CODEGEN_INTERFACES(
    SnoreSaber::Features::ScoreSubmission,
    ScoreSubmissionController,
    System::Object,
    Zenject::IInitializable*,
    System::IDisposable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionWorkflow*, _submissionWorkflow);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionService*, _scoreSubmissionService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::LeaderboardStatusController*, _leaderboardStatusController);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardPlayerScoreCache*, _playerScoreCache);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Live::Replay::LiveReplayStreamingService*, _liveReplayStreamingService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Live::Compete::Services::CompeteGameplayState*, _competeGameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, _isUploading);
    DECLARE_INSTANCE_FIELD_PRIVATE(int, _visibleUploadCount);
    DECLARE_CTOR(ctor,
                 SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionWorkflow* submissionWorkflow,
                 SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionService* scoreSubmissionService,
                 SnoreSaber::Features::Leaderboards::LeaderboardStatusController* leaderboardStatusController,
                 SnoreSaber::Features::Leaderboards::Services::LeaderboardPlayerScoreCache* playerScoreCache,
                 SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                 SnoreSaber::Features::Live::Replay::LiveReplayStreamingService* liveReplayStreamingService,
                 SnoreSaber::Features::Live::Compete::Services::CompeteGameplayState* competeGameplayState);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
    void HandleLevelFinished(StringW gameMode,
                             GlobalNamespace::BeatmapLevel* beatmapLevel,
                             GlobalNamespace::BeatmapKey beatmapKey,
                             GlobalNamespace::LevelCompletionResults* levelCompletionResults,
                             bool practicing,
                             float playOutcomeTime);
    void SubmitScore(GlobalNamespace::BeatmapLevel* beatmapLevel,
                     GlobalNamespace::BeatmapKey beatmapKey,
                     GlobalNamespace::LevelCompletionResults* levelCompletionResults,
                     float playOutcomeTime,
                     std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride,
                     bool visibleUpload);

  public:
    bool IsUploading() const;
    void HandleStandardLevelFinished(GlobalNamespace::StandardLevelScenesTransitionSetupDataSO* transition, GlobalNamespace::LevelCompletionResults* levelCompletionResults);
    void HandleMultiplayerLevelFinished(GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO* transition, GlobalNamespace::MultiplayerResultsData* multiplayerResultsData);
};
