#pragma once

#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp>
#include <GlobalNamespace/MultiplayerResultsData.hpp>
#include <GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp>

#include "Features/ScoreSubmission/Services/ScoreSubmissionRegistry.hpp"
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::ScoreSubmission::Services, ScoreSubmissionService, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionRegistry*, _registry);
    DECLARE_CTOR(ctor, SnoreSaber::Features::ScoreSubmission::Services::ScoreSubmissionRegistry* registry);

  public:
    using StandardCallback = std::function<void(GlobalNamespace::StandardLevelScenesTransitionSetupDataSO*, GlobalNamespace::LevelCompletionResults*)>;
    using MultiplayerCallback = std::function<void(GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO*, GlobalNamespace::MultiplayerResultsData*)>;

    void RegisterCallbacks(StandardCallback standardCallback, MultiplayerCallback multiplayerCallback);
    void ClearCallbacks();
    bool IsEnabled() const;
    void SetEnabled(bool enabled);
    void SuspendForReplay();
    void ResumeAfterReplay();
    void HandleStandardLevelFinished(GlobalNamespace::StandardLevelScenesTransitionSetupDataSO* transition, GlobalNamespace::LevelCompletionResults* levelCompletionResults);
    void HandleMultiplayerLevelFinished(GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO* transition, GlobalNamespace::MultiplayerResultsData* multiplayerResultsData);

  private:
    bool ShouldHandle() const;

    StandardCallback _standardCallback;
    MultiplayerCallback _multiplayerCallback;
    bool _subscribed = true;
};
