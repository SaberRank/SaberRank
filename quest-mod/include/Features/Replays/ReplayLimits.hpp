#pragma once

#include <cstddef>

namespace SnoreSaber::ReplaySystem::ReplayLimits
{
    constexpr std::size_t MaxCompressedReplayBytes = 128ull * 1024ull * 1024ull;
    constexpr std::size_t MaxDecompressedReplayBytes = 512ull * 1024ull * 1024ull;
}
