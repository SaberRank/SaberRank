#pragma once

#include "Features/Live/Protocol/Generated/Common.hpp"

#include <vector>

namespace SnoreSaber::Features::Live::Ludus::Services::LudusInstalledMods
{
    std::vector<::SnoreSaber::Live::V1::LiveMod> List();
}
