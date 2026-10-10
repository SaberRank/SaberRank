#pragma once

#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    enum class LeaderboardScreenStatus
    {
        Loading,
        Loaded,
        Empty,
        NoLeaderboard,
        NoPlayerScore,
        Error,
    };

    struct LeaderboardScreenState
    {
        LeaderboardScreenStatus status = LeaderboardScreenStatus::Error;
        std::optional<InternalLeaderboard> leaderboard;
        int playerScoreIndex = -1;
        std::string rankedStatus;
        std::string errorText;
        bool showRefreshButton = true;
        bool canPageUp = false;
        bool canPageDown = false;

        bool IsLoaded() const;

        static LeaderboardScreenState Loading(int page, bool canPage);
        static LeaderboardScreenState Loaded(InternalLeaderboard leaderboard, int playerScoreIndex, std::string rankedStatus, bool canPage, int page);
        static LeaderboardScreenState Failed(LeaderboardScreenStatus status, std::string errorText, bool showRefreshButton, std::optional<InternalLeaderboard> leaderboard,
                                             std::string rankedStatus, bool canPage, int page);
    };
}
