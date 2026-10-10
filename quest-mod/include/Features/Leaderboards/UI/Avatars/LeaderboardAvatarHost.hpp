#pragma once

#include "Features/Leaderboards/Domain/InternalLeaderboard.hpp"
#include "Features/Players/Profile/ProfilePictureView.hpp"

#include <System/Threading/CancellationToken.hpp>
#include <cstddef>
#include <vector>

namespace SnoreSaber::UI::Other
{
    class LeaderboardAvatarHost
    {
        std::vector<ProfilePictureView> avatarViews;

    public:
        static constexpr std::size_t MaxAvatarCount = 10;

        void AddAvatar(ProfilePictureView avatarView);
        void Reset();
        void ClearAvatars();
        void LoadAvatars(const SnoreSaber::Data::InternalLeaderboard& leaderboard, System::Threading::CancellationToken cancellationToken);
        std::size_t Count() const;
    };
}
