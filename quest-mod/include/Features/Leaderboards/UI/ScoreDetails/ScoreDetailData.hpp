#pragma once

#include "Features/Leaderboards/Domain/ScoreMap.hpp"

#include <string>

namespace SnoreSaber::UI::Other
{
    struct ScoreDetailData
    {
        SnoreSaber::Data::ScoreMap score;
        std::string playerId;
        std::u16string playerNameText;
        std::string deviceHmdText;
        std::string deviceControllerLeftText;
        std::string deviceControllerRightText;
        std::string scoreText;
        std::string ppText;
        std::string maxComboText;
        std::string fullComboText;
        std::string badCutsText;
        std::string missedNotesText;
        std::string modifiersText;
        std::string timeSetText;
        std::string crownImage;
        std::string crownDescription;
        bool hasReplay = false;

        bool HasCrown() const;
        static ScoreDetailData Create(const SnoreSaber::Data::ScoreMap& score);
    };
}
