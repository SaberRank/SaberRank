#pragma once

#include "Features/Leaderboards/Domain/LeaderboardPlayer.hpp"

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    struct Score
    {
        Score() = default;

        int id;
        LeaderboardPlayer leaderboardPlayerInfo;
        int rank;
        int baseScore;
        int modifiedScore;
        double pp;
        double weight;
        std::string modifiers;
        double multiplier;
        int badCuts;
        int missedNotes;
        int maxCombo;
        bool fullCombo;
        int hmd;
        std::optional<std::string> deviceHmd;
        std::optional<std::string> deviceControllerLeft;
        std::optional<std::string> deviceControllerRight;
        bool personalBest = false;
        std::string playOutcome;
        std::optional<double> playOutcomeTime;
        bool hasReplay;
        std::string timeSet;
    };
}
