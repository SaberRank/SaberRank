#include "Features/ScoreSubmission/Services/ScoreSubmissionRegistry.hpp"

DEFINE_TYPE(SnoreSaber::Features::ScoreSubmission::Services, ScoreSubmissionRegistry);

namespace SnoreSaber::Features::ScoreSubmission::Services
{
    void ScoreSubmissionRegistry::ctor()
    {
        INVOKE_CTOR();
        _enabled = true;
    }

    bool ScoreSubmissionRegistry::IsEnabled() const
    {
        return _enabled;
    }

    void ScoreSubmissionRegistry::SetEnabled(bool enabled)
    {
        _enabled = enabled;
    }
}
