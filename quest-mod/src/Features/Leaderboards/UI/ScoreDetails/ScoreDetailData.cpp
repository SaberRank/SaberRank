#include "Features/Leaderboards/UI/ScoreDetails/ScoreDetailData.hpp"

#include "Core/Presentation/PlayerPresentation.hpp"
#include "Features/Leaderboards/UI/ScoreDetails/ScoreAgeFormatter.hpp"
#include "Utils/NumberFormatter.hpp"

#include <fmt/format.h>
#include <optional>

namespace SnoreSaber::UI::Other
{
    namespace
    {
        std::string GetLegacyDeviceName(int id)
        {
            if (id == 0) return "Unknown";
            if (id == 1) return "Oculus Rift CV1";
            if (id == 2) return "HTC VIVE";
            if (id == 4) return "HTC VIVE Pro";
            if (id == 8) return "Windows Mixed Reality";
            if (id == 16) return "Oculus Rift S";
            if (id == 32) return "Oculus Quest";
            if (id == 64) return "Valve Index";
            if (id == 128) return "HTC VIVE Cosmos";
            return "Unknown";
        }

        std::string DeviceText(const std::optional<std::string>& device)
        {
            return device.has_value() && !device->empty() ? device.value() : "N/A";
        }

        std::string CountText(int value)
        {
            return value != 0 ? SnoreSaber::Utils::FormatInteger(value) : "N/A";
        }

        std::string FailureCountText(int value, bool hasScoreDetails)
        {
            if (!hasScoreDetails)
            {
                return "N/A";
            }
            if (value > 0)
            {
                return fmt::format("<color=#FF0000>{}</color>", SnoreSaber::Utils::FormatInteger(value));
            }
            return SnoreSaber::Utils::FormatInteger(value);
        }
    }

    bool ScoreDetailData::HasCrown() const
    {
        return !crownImage.empty();
    }

    ScoreDetailData ScoreDetailData::Create(const SnoreSaber::Data::ScoreMap& scoreMap)
    {
        const auto& score = scoreMap.score;
        bool hasScoreDetails = score.maxCombo != 0;
        bool givesPP = scoreMap.parent.leaderboard.status == SnoreSaber::Data::LeaderboardStatus::Ranked;
        auto crownDetails = SnoreSaber::Core::Presentation::PlayerPresentation::GetCrownDetails(score.leaderboardPlayerInfo.id.value_or(""));

        ScoreDetailData data;
        data.score = scoreMap;
        data.playerId = score.leaderboardPlayerInfo.id.value_or("");
        data.playerNameText = score.leaderboardPlayerInfo.name.value_or(u"Unknown") + u"'s Score";
        data.deviceHmdText = score.deviceHmd.value_or(GetLegacyDeviceName(score.hmd));
        data.deviceControllerLeftText = DeviceText(score.deviceControllerLeft);
        data.deviceControllerRightText = DeviceText(score.deviceControllerRight);
        data.scoreText = fmt::format("{} (<color=#FFD42A>{:.2f}%</color>)", SnoreSaber::Utils::FormatInteger(score.modifiedScore), scoreMap.accuracy);
        data.ppText = givesPP ? fmt::format("<color=#6772E5>{:.2f} ZZ</color>", score.pp) : "N/A";
        data.maxComboText = CountText(score.maxCombo);
        data.fullComboText = hasScoreDetails ? (score.fullCombo ? "<color=#9EDBB1>Yes</color>" : "<color=#FF0000>No</color>") : "N/A";
        data.badCutsText = FailureCountText(score.badCuts, hasScoreDetails);
        data.missedNotesText = FailureCountText(score.missedNotes, hasScoreDetails);
        data.modifiersText = scoreMap.modifierText;
        data.timeSetText = ScoreAgeFormatter::FormatAgo(score.timeSet);
        data.crownImage = crownDetails.image;
        data.crownDescription = crownDetails.description;
        data.hasReplay = scoreMap.HasReplay();
        return data;
    }
}
