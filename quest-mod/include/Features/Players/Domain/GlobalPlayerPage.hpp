#pragma once

#include "Features/Players/Domain/GlobalPlayerScope.hpp"
#include "Features/Players/Domain/Player.hpp"

#include <vector>

namespace SnoreSaber::Data
{
    struct GlobalPlayerPage
    {
        GlobalPlayerScope scope = GlobalPlayerScope::Global;
        int page = 1;
        std::vector<Player> players;
    };
}
