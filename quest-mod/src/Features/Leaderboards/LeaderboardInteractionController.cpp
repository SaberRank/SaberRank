#include "Features/Leaderboards/LeaderboardInteractionController.hpp"

DEFINE_TYPE(SnoreSaber::Features::Leaderboards, LeaderboardInteractionController);

namespace SnoreSaber::Features::Leaderboards
{
    void LeaderboardInteractionController::ctor(Services::LeaderboardScreenSession* leaderboardSession)
    {
        INVOKE_CTOR();
        _leaderboardSession = leaderboardSession;
    }

    void LeaderboardInteractionController::SelectScope(GlobalNamespace::PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry)
    {
        _leaderboardSession->SelectScope(scope, filterAroundCountry);
    }

    void LeaderboardInteractionController::PageUp()
    {
        _leaderboardSession->PageUp();
    }

    void LeaderboardInteractionController::PageDown()
    {
        _leaderboardSession->PageDown();
    }

    bool LeaderboardInteractionController::CanPageUp() const
    {
        return _leaderboardSession->Page() > 1;
    }
}
