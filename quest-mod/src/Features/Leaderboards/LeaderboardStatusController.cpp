#include "Features/Leaderboards/LeaderboardStatusController.hpp"

DEFINE_TYPE(SnoreSaber::Features::Leaderboards, LeaderboardStatusController);

namespace SnoreSaber::Features::Leaderboards
{
    void LeaderboardStatusController::ctor(LeaderboardPresentationController* presentationController)
    {
        INVOKE_CTOR();
        _presentationController = presentationController;
    }

    void LeaderboardStatusController::ApplySubmissionStatus(SnoreSaber::Data::Private::ScoreSubmissionStatus status)
    {
        using SnoreSaber::Data::Private::ScoreUploadStatus;

        switch (status.status)
        {
            case ScoreUploadStatus::Packaging:
            case ScoreUploadStatus::Uploading:
            case ScoreUploadStatus::Retrying:
                _presentationController->SetUploadState(true, false, "");
                return;
            case ScoreUploadStatus::Success:
                _presentationController->SetUploadState(false, true, "");
                return;
            case ScoreUploadStatus::Error:
                _presentationController->SetUploadState(false, false, "<color=#fc8181>" + status.message + "</color>");
                return;
            case ScoreUploadStatus::Done:
                return;
        }
    }
}
