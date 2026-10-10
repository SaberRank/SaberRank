#pragma once

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>

#include "Core/Gameplay/SnoreSaberPlayOutcome.hpp"
#include "Core/SnoreSaberRuntimeInfo.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <optional>
#include <string>

namespace SnoreSaber::Features::ScoreSubmission::Services
{
    struct ScoreUploadPayload
    {
        std::string encryptedScoreData;
        std::string uploadVersionHash;
        int multipliedScore = 0;
        std::string serializedScoreData;
    };
}

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::ScoreSubmission::Services, ScoreUploadPayloadBuilder, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Core::SnoreSaberRuntimeInfo*, _runtimeInfo);
    DECLARE_CTOR(ctor,
                 SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                 SnoreSaber::Core::SnoreSaberRuntimeInfo* runtimeInfo);

  public:
    ScoreUploadPayload Build(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, GlobalNamespace::LevelCompletionResults* levelCompletionResults, float playOutcomeTime, std::optional<SnoreSaber::Core::Gameplay::SnoreSaberPlayOutcome> playOutcomeOverride);
    ScoreUploadPayload RebuildWithCurrentSession(const ScoreUploadPayload& payload);
    std::string GetVersionHash();
};
