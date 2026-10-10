#pragma once

#include <GlobalNamespace/BeatmapKey.hpp>
#include <beatsaber-hook/shared/utils/typedefs-string.hpp>
#include <optional>
#include <string>

namespace SnoreSaber::Utils::SnoreSaberBeatmapKey
{
    bool IsCustomLevel(GlobalNamespace::BeatmapKey beatmapKey);
    bool IsCustomLevelId(const std::string& levelId);
    bool IsWip(GlobalNamespace::BeatmapKey beatmapKey);
    bool IsWipLevelId(const std::string& levelId);
    bool IsSupported(GlobalNamespace::BeatmapKey beatmapKey);
    bool IsSupportedLevelId(const std::string& levelId);
    std::optional<std::string> TryGetSongHash(GlobalNamespace::BeatmapKey beatmapKey);
    std::optional<std::string> TryGetSongHash(const std::string& levelId);
    std::string GetSongHash(GlobalNamespace::BeatmapKey beatmapKey);
} // namespace SnoreSaber::Utils::SnoreSaberBeatmapKey
