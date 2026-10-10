#pragma once

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    struct LeaderboardPlayer
    {
        LeaderboardPlayer() = default;

        std::optional<std::string> id;
        std::optional<std::u16string> name;
        std::string profilePicture;
        std::optional<std::string> country;
        std::optional<int> permissions;
        std::optional<std::string> role;
    };
}
