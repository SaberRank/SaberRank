#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include <memory>
#include <vector>

namespace SnoreSaber::ReplaySystem::ReplayFileCodec
{
    std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> Read(const std::vector<char>& replay);
    std::vector<char> Write(const std::shared_ptr<SnoreSaber::Data::Private::ReplayFile>& replay);
}
