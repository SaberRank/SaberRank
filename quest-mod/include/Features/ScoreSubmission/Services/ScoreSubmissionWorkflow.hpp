#pragma once

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>

#include "Core/Gameplay/SnoreSaberPlayOutcome.hpp"
#include "Features/Leaderboards/Services/MaxScoreCache.hpp"
#include "Features/Leaderboards/Services/LeaderboardQueryService.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Features/Replays/ReplayStorageService.hpp"
#include "Features/ScoreSubmission/Domain/ScoreUploadResult.hpp"
#include "Features/ScoreSubmission/Services/ScoreUploadPayloadBuilder.hpp"
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>
#include <optional>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::ScoreSubmission::Services, ScoreSubmissionWorkflow, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::ScoreSubmission::Services::ScoreUploadPayloadBuilder*, _payloadBuilder);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Utils::MaxScoreCache*, _maxScoreCache);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService*, _leaderboardQueryService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::ReplaySystem::ReplayStorageService*, _replayStorageService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_CTOR(ctor,
                 SnoreSaber::Features::ScoreSubmission::Services::ScoreUploadPayloadBuilder* payloadBuilder,
                 SnoreSaber::Utils::MaxScoreCache* maxScoreCache,
                 SnoreSaber::Features::Leaderboards::Services::LeaderboardQueryService* leaderboardQueryService,
                 SnoreSaber::ReplaySystem::ReplayStorageService* replayStorageService,
                 SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService);

  public:
    using StatusChanged = std::function<void(SnoreSaber::Data::Private::ScoreSubmissionStatus)>;
    using Completed = std::function<void(SnoreSaber::Data::Private::ScoreUploadResult, bool statusVisible)>;

    void SubmitScore(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, GlobalNamespace::LevelCompletionResults* levelCompletionResults, float playOutcomeTime, std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride, bool saveLocalReplay, StatusChanged statusChanged, Completed completed);
    void DiscardReplay();
    void WriteReplayOnly(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, GlobalNamespace::LevelCompletionResults* levelCompletionResults, float playOutcomeTime);
};
