#pragma once

extern "C"
{
    #include "Utils/lzma/pavlov/LzmaUtil.h"
}
#include <cstddef>
#include <vector>

namespace LZMA
{
    // both of these are highly non-threadsafe, if two of these calls (even different ones) run at the same time it *will* mess things up.
    bool lzmaDecompress(const std::vector<char> &in, std::vector<char> &out, std::size_t maxOutputSize);
    bool lzmaDecompress(const std::vector<char> &in, std::vector<char> &out);
    bool lzmaCompress(const std::vector<char> &in, std::vector<char> &out);
}
