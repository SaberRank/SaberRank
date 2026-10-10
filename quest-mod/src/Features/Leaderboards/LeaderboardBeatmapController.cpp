#include "Features/Leaderboards/LeaderboardBeatmapController.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"

DEFINE_TYPE(SnoreSaber::Features::Leaderboards, LeaderboardBeatmapController);

namespace SnoreSaber::Features::Leaderboards
{
    void LeaderboardBeatmapController::ctor(Services::LeaderboardScreenSession* leaderboardSession)
    {
        INVOKE_CTOR();
        _leaderboardSession = leaderboardSession;
    }

    void LeaderboardBeatmapController::OnLeaderboardSet(GlobalNamespace::BeatmapKey beatmapKey)
    {
        if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(beatmapKey))
        {
            _leaderboardSession->ClearBeatmap();
            return;
        }

        _leaderboardSession->SetBeatmap(beatmapKey);
    }
}
