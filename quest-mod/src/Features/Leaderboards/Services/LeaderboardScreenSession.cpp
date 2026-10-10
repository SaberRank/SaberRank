#include "Features/Leaderboards/Services/LeaderboardScreenSession.hpp"

#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"

using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::Features::Leaderboards::Services, LeaderboardScreenSession);

namespace SnoreSaber::Features::Leaderboards::Services
{
    void LeaderboardScreenSession::ctor(LeaderboardScreenLoader* leaderboardLoader)
    {
        INVOKE_CTOR();
        _leaderboardLoader = leaderboardLoader;
        Reset();
    }

    void LeaderboardScreenSession::Reset()
    {
        _page = 1;
        _filterAroundCountry = false;
        _currentRefreshId = -1;
    }

    void LeaderboardScreenSession::SetBeatmap(BeatmapKey beatmapKey)
    {
        if (!SnoreSaber::Utils::SnoreSaberBeatmapKey::IsSupported(beatmapKey))
        {
            ClearBeatmap();
            return;
        }

        RefreshFromFirstPage();
    }

    void LeaderboardScreenSession::ClearBeatmap()
    {
        Reset();
    }

    void LeaderboardScreenSession::SelectScope(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry)
    {
        (void)scope;
        _filterAroundCountry = filterAroundCountry;
        RefreshFromFirstPage();
    }

    void LeaderboardScreenSession::RefreshFromFirstPage()
    {
        _page = 1;
    }

    void LeaderboardScreenSession::PageUp()
    {
        if (_page <= 1)
        {
            return;
        }

        _page--;
    }

    void LeaderboardScreenSession::PageDown()
    {
        _page++;
    }

    int LeaderboardScreenSession::Page()
    {
        return _page;
    }

    bool LeaderboardScreenSession::FilterAroundCountry()
    {
        return _filterAroundCountry;
    }

    int LeaderboardScreenSession::BeginRefresh(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry)
    {
        (void)scope;
        _filterAroundCountry = filterAroundCountry;
        _currentRefreshId = _currentRefreshId + 1;
        return _currentRefreshId;
    }

    bool LeaderboardScreenSession::IsCurrentRefresh(int refreshId)
    {
        return _currentRefreshId == refreshId;
    }

    bool LeaderboardScreenSession::CanPageScope(PlatformLeaderboardsModel::ScoresScope scope, bool filterAroundCountry) const
    {
        return _leaderboardLoader->CanPageScope(scope, filterAroundCountry);
    }
}
