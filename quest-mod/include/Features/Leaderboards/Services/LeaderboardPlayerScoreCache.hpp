#pragma once

#include "Features/Leaderboards/Domain/LeaderboardQuery.hpp"
#include "Features/Leaderboards/Domain/Score.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <optional>
#include <string>
#include <unordered_map>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards::Services, LeaderboardPlayerScoreCache, Il2CppObject) {
    DECLARE_DEFAULT_CTOR();

    std::unordered_map<std::string, SnoreSaber::Data::Score> _scores;

  public:
    void Remember(const SnoreSaber::Data::LeaderboardQuery& query, const std::string& playerId, const std::optional<SnoreSaber::Data::Score>& score);
    bool TryGet(GlobalNamespace::BeatmapKey beatmapKey, const std::string& playerId, SnoreSaber::Data::Score& score) const;
};
