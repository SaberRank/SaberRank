#pragma once

#include "Features/Players/Domain/Player.hpp"

#include <string>
#include <vector>

namespace SnoreSaber::UI::Other
{
    struct ProfileCrownData
    {
        std::string image;
        std::string description;

        bool HasCrown() const;
    };

    struct ProfileBadgeData
    {
        std::string image;
        std::string description;
    };

    struct ProfileDetailData
    {
        SnoreSaber::Data::Player player;
        std::u16string displayName;
        std::string avatar;
        std::string rankText;
        std::string ppText;
        std::string rankedAccuracyText;
        std::string totalScoreText;
        bool usesFurryFont = false;
        ProfileCrownData crown;
        std::vector<ProfileBadgeData> badges;

        static ProfileDetailData Create(const SnoreSaber::Data::Player& player);
    };
}
