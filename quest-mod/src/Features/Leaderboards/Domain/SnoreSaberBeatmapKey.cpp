#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"

#include "Utils/StringUtils.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>

namespace SnoreSaber::Utils::SnoreSaberBeatmapKey
{
    namespace
    {
        constexpr std::string_view CustomLevelPrefix = "custom_level_";
        constexpr std::string_view WipLevelSuffix = " WIP";
        constexpr std::string_view WipLevelSegment = "_WIP";

        bool ContainsCaseInsensitive(std::string value, std::string_view needle)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
                return std::tolower(c);
            });

            std::string lowerNeedle(needle);
            std::transform(lowerNeedle.begin(), lowerNeedle.end(), lowerNeedle.begin(), [](unsigned char c) {
                return std::tolower(c);
            });

            return value.find(lowerNeedle) != std::string::npos;
        }

        bool StartsWith(std::string_view value, std::string_view prefix)
        {
            return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
        }

        std::string LevelId(GlobalNamespace::BeatmapKey beatmapKey)
        {
            return beatmapKey.levelId;
        }
    } // namespace

    bool IsCustomLevel(GlobalNamespace::BeatmapKey beatmapKey)
    {
        return IsCustomLevelId(LevelId(beatmapKey));
    }

    bool IsCustomLevelId(const std::string& levelId)
    {
        return !levelId.empty() && StartsWith(levelId, CustomLevelPrefix);
    }

    bool IsWip(GlobalNamespace::BeatmapKey beatmapKey)
    {
        std::string levelId = LevelId(beatmapKey);
        return IsCustomLevelId(levelId) && IsWipLevelId(levelId);
    }

    bool IsWipLevelId(const std::string& levelId)
    {
        return !levelId.empty() && (ContainsCaseInsensitive(levelId, WipLevelSuffix) || ContainsCaseInsensitive(levelId, WipLevelSegment));
    }

    std::optional<std::string> TryGetSongHash(GlobalNamespace::BeatmapKey beatmapKey)
    {
        return TryGetSongHash(LevelId(beatmapKey));
    }

    std::optional<std::string> TryGetSongHash(const std::string& levelId)
    {
        if (levelId.empty())
            return std::nullopt;

        if (IsCustomLevelId(levelId) && IsWipLevelId(levelId))
            return std::nullopt;

        std::string hash = StringUtils::GetFormattedHash(levelId);
        if (hash.empty())
            return std::nullopt;

        return hash;
    }

    bool IsSupported(GlobalNamespace::BeatmapKey beatmapKey)
    {
        return TryGetSongHash(beatmapKey).has_value();
    }

    bool IsSupportedLevelId(const std::string& levelId)
    {
        return TryGetSongHash(levelId).has_value();
    }

    std::string GetSongHash(GlobalNamespace::BeatmapKey beatmapKey)
    {
        auto songHash = TryGetSongHash(beatmapKey);
        if (!songHash.has_value())
            throw std::invalid_argument("Unsupported SnoreSaber level id: " + LevelId(beatmapKey));

        return songHash.value();
    }
} // namespace SnoreSaber::Utils::SnoreSaberBeatmapKey
