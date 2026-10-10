#include "Features/ScoreSubmission/Services/ScoreSubmissionService.hpp"

#include "Features/Replays/ReplayStateRegistry.hpp"

DEFINE_TYPE(SnoreSaber::Features::ScoreSubmission::Services, ScoreSubmissionService);

namespace SnoreSaber::Features::ScoreSubmission::Services
{
    void ScoreSubmissionService::ctor(ScoreSubmissionRegistry* registry)
    {
        INVOKE_CTOR();
        _registry = registry;
    }

    void ScoreSubmissionService::RegisterCallbacks(StandardCallback standardCallback, MultiplayerCallback multiplayerCallback)
    {
        ClearCallbacks();
        _standardCallback = std::move(standardCallback);
        _multiplayerCallback = std::move(multiplayerCallback);
        ResumeAfterReplay();
    }

    void ScoreSubmissionService::ClearCallbacks()
    {
        _standardCallback = nullptr;
        _multiplayerCallback = nullptr;
    }

    bool ScoreSubmissionService::IsEnabled() const
    {
        return _registry && _registry->IsEnabled();
    }

    void ScoreSubmissionService::SetEnabled(bool enabled)
    {
        if (!_registry)
        {
            return;
        }

        _registry->SetEnabled(enabled);
    }

    void ScoreSubmissionService::SuspendForReplay()
    {
        _subscribed = false;
    }

    void ScoreSubmissionService::ResumeAfterReplay()
    {
        _subscribed = true;
    }

    void ScoreSubmissionService::HandleStandardLevelFinished(GlobalNamespace::StandardLevelScenesTransitionSetupDataSO* transition, GlobalNamespace::LevelCompletionResults* levelCompletionResults)
    {
        if (!ShouldHandle() || !_standardCallback)
        {
            return;
        }

        _standardCallback(transition, levelCompletionResults);
    }

    void ScoreSubmissionService::HandleMultiplayerLevelFinished(GlobalNamespace::MultiplayerLevelScenesTransitionSetupDataSO* transition, GlobalNamespace::MultiplayerResultsData* multiplayerResultsData)
    {
        if (!ShouldHandle() || !_multiplayerCallback)
        {
            return;
        }

        _multiplayerCallback(transition, multiplayerResultsData);
    }

    bool ScoreSubmissionService::ShouldHandle() const
    {
        return _subscribed && IsEnabled() && !SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled();
    }
}
