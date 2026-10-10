#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"

#include <memory>
#include <vector>

namespace SnoreSaber::Data::Private::ReplayWriter
{
    std::vector<char> Write(std::shared_ptr<ReplayFile> file);
} // namespace SnoreSaber::Data::Private::ReplayWriter
