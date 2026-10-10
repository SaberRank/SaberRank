#pragma once

#include <optional>
#include <string>

namespace SnoreSaber::Data
{
    enum class PlayerQueryScope
    {
        Global,
        AroundPlayer,
        Friends,
        Country,
        Region,
        Countries,
    };

    struct PlayerListQuery
    {
        int page = 1;
        int limit = 5;
        PlayerQueryScope scope = PlayerQueryScope::Global;
        std::string countries;
        std::optional<int> realmId;
    };
}
