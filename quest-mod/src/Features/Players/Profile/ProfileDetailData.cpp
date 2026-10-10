#include "Features/Players/Profile/ProfileDetailData.hpp"

#include "Core/Presentation/PlayerPresentation.hpp"
#include "Utils/NumberFormatter.hpp"

#include <cmath>
#include <fmt/format.h>

namespace SnoreSaber::UI::Other
{
    bool ProfileCrownData::HasCrown() const
    {
        return !image.empty();
    }

    ProfileDetailData ProfileDetailData::Create(const SnoreSaber::Data::Player& player)
    {
        double averageRankedAccuracy = player.scoreStats.has_value() ? player.scoreStats->averageRankedAccuracy : 0.0;
        long totalScore = player.scoreStats.has_value() ? player.scoreStats->totalScore : 0;
        auto crownDetails = SnoreSaber::Core::Presentation::PlayerPresentation::GetCrownDetails(player.id);

        ProfileDetailData data;
        data.player = player;
        data.displayName = player.name;
        data.avatar = player.profilePicture;
        data.rankText = "#" + SnoreSaber::Utils::FormatInteger(player.rank);
        data.ppText = fmt::format("<color=#6772E5>{} ZZ</color>", SnoreSaber::Utils::FormatInteger(static_cast<int64_t>(std::llround(player.pp))));
        data.rankedAccuracyText = fmt::format("{:.2f}%", averageRankedAccuracy);
        data.totalScoreText = SnoreSaber::Utils::FormatInteger(totalScore);
        data.usesFurryFont = SnoreSaber::Core::Presentation::PlayerPresentation::UsesFurryFont(player.id);
        data.crown = { crownDetails.image, crownDetails.description };

        for (auto const& badge : player.badges)
        {
            data.badges.push_back({ badge.image, badge.description });
        }

        return data;
    }
}
