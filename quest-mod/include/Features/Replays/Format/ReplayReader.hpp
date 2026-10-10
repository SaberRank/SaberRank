#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"

#include <memory>
#include <vector>

namespace SnoreSaber::Data::Private::ReplayReader
{
    std::shared_ptr<ReplayFile> Read(const std::vector<char>& replayData);
} // namespace SnoreSaber::Data::Private::ReplayReader
