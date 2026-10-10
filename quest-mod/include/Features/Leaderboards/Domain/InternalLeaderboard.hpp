#pragma once

#include "Features/Leaderboards/Domain/LeaderboardMap.hpp"
#include "Features/Leaderboards/Domain/LeaderboardSnapshot.hpp"
#include <GlobalNamespace/LeaderboardTableView.hpp>
#include <GlobalNamespace/PlatformLeaderboardsModel.hpp>
#include <beatsaber-hook/shared/utils/typedefs.h>
#include <optional>
#include <string>
#include <vector>

using namespace GlobalNamespace;
namespace SnoreSaber::Data
{
    struct InternalLeaderboard
    {
        InternalLeaderboard();
        InternalLeaderboard(ListW<LeaderboardTableView::ScoreData*> _leaderboardItems,
                            std::vector<std::string> _profilePictures,
                            std::optional<LeaderboardSnapshot> _leaderboard = std::nullopt,
                            std::optional<LeaderboardMap> _leaderboardMap = std::nullopt);

        ListW<LeaderboardTableView::ScoreData*> leaderboardItems;
        std::vector<std::string> profilePictures;
        std::optional<LeaderboardSnapshot> leaderboard;
        std::optional<LeaderboardMap> leaderboardMap;
        bool leaderboardNotFound = false;
    };
}
