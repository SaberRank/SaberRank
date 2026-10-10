#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace SnoreSaber::Data::Private::HsvReplayConfigCodec
{
    inline constexpr int MaxJsonBytes = 32 * 1024;
    inline constexpr int MaxPayloadBytes = 8 * 1024;

    bool TryEncodeJson(std::string_view json, std::vector<char>& payload, std::string& failure);
} // namespace SnoreSaber::Data::Private::HsvReplayConfigCodec
