#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace SnoreSaber::Data::Private::ReplayExtensionPayloads
{
    inline constexpr std::string_view PlaySettingsExtension = "snoresaber.play-settings";
    inline constexpr std::string_view PauseEventsExtension = "snoresaber.pause-events";
    inline constexpr std::string_view WallEventsExtension = "snoresaber.wall-events";
    inline constexpr std::string_view ControllerOffsetsExtension = "snoresaber.controller-offsets";
    inline constexpr std::string_view HsvConfigExtension = "snoresaber.hsv-config";

    struct ReplayExtensionEntry
    {
        std::string Id;
        int Version;
        std::vector<char> Payload;
    };

    bool HasFileExtensions(const ReplayFile& file);
    std::vector<ReplayExtensionEntry> CreateFileExtensions(const ReplayFile& file);
    std::vector<ReplayExtensionEntry> CreateStartExtensions(const Metadata& metadata, const std::vector<char>& hsvConfig);
    ReplayExtensionEntry CreatePauseEvents(const std::vector<PauseEvent>& pauseEvents);
    ReplayExtensionEntry CreateWallEvents(const std::vector<WallEvent>& wallEvents);
} // namespace SnoreSaber::Data::Private::ReplayExtensionPayloads
