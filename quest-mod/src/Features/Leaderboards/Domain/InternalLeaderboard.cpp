#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"

using namespace GlobalNamespace;

namespace SnoreSaber::Data
{
    InternalLeaderboard::InternalLeaderboard(ListW<LeaderboardTableView::ScoreData*> _leaderboardItems,
                                             std::vector<std::string> _profilePictures,
                                             std::optional<LeaderboardSnapshot> _leaderboard,
                                             std::optional<LeaderboardMap> _leaderboardMap)
    {
        leaderboardItems = _leaderboardItems;
        profilePictures = _profilePictures;
        leaderboard = _leaderboard;
        leaderboardMap = _leaderboardMap;
    }
    InternalLeaderboard::InternalLeaderboard() {}
}
