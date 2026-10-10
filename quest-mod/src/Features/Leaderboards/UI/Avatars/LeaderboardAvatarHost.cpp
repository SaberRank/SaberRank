#include "Features/Leaderboards/UI/Avatars/LeaderboardAvatarHost.hpp"

#include <algorithm>
#include <utility>

namespace SnoreSaber::UI::Other
{
    void LeaderboardAvatarHost::AddAvatar(ProfilePictureView avatarView)
    {
        if (avatarViews.size() >= MaxAvatarCount)
        {
            return;
        }

        avatarViews.emplace_back(std::move(avatarView));
        avatarViews.back().Parsed();
    }

    void LeaderboardAvatarHost::Reset()
    {
        avatarViews.clear();
    }

    void LeaderboardAvatarHost::ClearAvatars()
    {
        for (auto& avatarView : avatarViews)
        {
            avatarView.ClearSprite();
        }
    }

    void LeaderboardAvatarHost::LoadAvatars(const SnoreSaber::Data::InternalLeaderboard& leaderboard, System::Threading::CancellationToken cancellationToken)
    {
        ClearAvatars();

        auto avatarCount = std::min(leaderboard.profilePictures.size(), avatarViews.size());
        for (std::size_t i = 0; i < avatarCount; ++i)
        {
            avatarViews[i].SetProfileImage(leaderboard.profilePictures[i], static_cast<int>(i), cancellationToken);
        }
    }

    std::size_t LeaderboardAvatarHost::Count() const
    {
        return avatarViews.size();
    }
}
