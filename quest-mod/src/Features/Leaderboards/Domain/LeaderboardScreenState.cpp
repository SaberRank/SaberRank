#include "Features/Leaderboards/Domain/LeaderboardScreenState.hpp"

#include <utility>

namespace SnoreSaber::Data
{
    bool LeaderboardScreenState::IsLoaded() const
    {
        return status != LeaderboardScreenStatus::Loading;
    }

    LeaderboardScreenState LeaderboardScreenState::Loading(int page, bool canPage)
    {
        LeaderboardScreenState state;
        state.status = LeaderboardScreenStatus::Loading;
        state.canPageUp = canPage && page > 1;
        state.canPageDown = canPage;
        return state;
    }

    LeaderboardScreenState LeaderboardScreenState::Loaded(InternalLeaderboard leaderboard, int playerScoreIndex, std::string rankedStatus, bool canPage, int page)
    {
        LeaderboardScreenState state;
        state.status = LeaderboardScreenStatus::Loaded;
        state.leaderboard = std::move(leaderboard);
        state.playerScoreIndex = playerScoreIndex;
        state.rankedStatus = std::move(rankedStatus);
        state.canPageUp = canPage && page > 1;
        state.canPageDown = canPage;
        return state;
    }

    LeaderboardScreenState LeaderboardScreenState::Failed(LeaderboardScreenStatus status, std::string errorText, bool showRefreshButton, std::optional<InternalLeaderboard> leaderboard,
                                                          std::string rankedStatus, bool canPage, int page)
    {
        LeaderboardScreenState state;
        state.status = status;
        state.leaderboard = std::move(leaderboard);
        state.rankedStatus = std::move(rankedStatus);
        state.errorText = std::move(errorText);
        state.showRefreshButton = showRefreshButton;
        state.canPageUp = canPage && page > 1;
        state.canPageDown = canPage;
        return state;
    }
}
