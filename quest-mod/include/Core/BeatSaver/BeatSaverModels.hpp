#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace SnoreSaber::Core::BeatSaver
{
    struct BeatSaverMapMetadata
    {
        std::string songName;
        std::string songSubName;
        std::string songAuthorName;
        std::string levelAuthorName;
        std::optional<float> duration;
        std::optional<float> bpm;
    };

    struct BeatSaverUploader
    {
        std::string name;
    };

    struct BeatSaverDifficulty
    {
        std::string difficulty;
        std::string characteristic;
        std::optional<float> nps;
        std::optional<int> notes;
        std::optional<int> obstacles;
        std::optional<int> bombs;
        std::optional<float> njs;
        std::optional<float> offset;
    };

    struct BeatSaverVersion
    {
        std::string hash;
        std::string downloadUrl;
        std::string coverUrl;
        std::vector<BeatSaverDifficulty> diffs;
    };

    struct BeatSaverMap
    {
        std::string name;
        std::optional<BeatSaverMapMetadata> metadata;
        std::optional<BeatSaverUploader> uploader;
        std::vector<BeatSaverVersion> versions;

        static std::optional<BeatSaverMap> TryParse(std::string_view json);
    };
} // namespace SnoreSaber::Core::BeatSaver
