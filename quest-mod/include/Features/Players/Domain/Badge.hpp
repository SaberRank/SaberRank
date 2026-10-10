#pragma once

#include <string>

namespace SnoreSaber::Data
{
    struct Badge
    {
        Badge() = default;

        std::string description;
        std::string image;
    };
}
