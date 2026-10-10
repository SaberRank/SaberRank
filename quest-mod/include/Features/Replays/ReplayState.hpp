#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Utils/SafePtr.hpp"
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <memory>
#include <string>

namespace SnoreSaber::ReplaySystem
{
    struct ReplayState
    {
        SafePtr<GlobalNamespace::BeatmapLevel> currentBeatmapLevel;
        SafeValueType<GlobalNamespace::BeatmapKey> currentBeatmapKey;
        std::string currentModifiers;
        std::u16string currentPlayerName;
        bool isLegacyReplay = false;
        bool isPlaybackEnabled = false;
        std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> loadedReplayFile;

        void Reset();
        void BeginReplay(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, std::string modifiers, std::u16string playerName);
        void LoadReplay(std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> replay);
        void EndPlayback();
    };
}
