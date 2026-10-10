#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Utils/SafePtr.hpp"

#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <System/zzzz__Object_def.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <memory>
#include <string>
#include <string_view>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem, ReplayPlaybackContext, System::Object) {
    DECLARE_CTOR(ctor);

  public:
    void UseCurrentState();
    bool IsPlaybackEnabled() const;
    bool IsModernPlaybackEnabled() const;
    std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> GetReplayFile() const;
    GlobalNamespace::BeatmapLevel* GetBeatmapLevel() const;
    GlobalNamespace::BeatmapKey GetBeatmapKey() const;
    const std::u16string& GetPlayerName() const;
    const std::string& GetModifiers() const;
    float GetInitialTimeScale() const;
    bool HasModifier(std::string_view modifier) const;

  private:
    SafePtr<GlobalNamespace::BeatmapLevel> _beatmapLevel;
    SafeValueType<GlobalNamespace::BeatmapKey> _beatmapKey;
    std::u16string _playerName;
    std::string _modifiers;
    bool _isLegacyReplay = false;
    bool _isPlaybackEnabled = false;
    std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> _replayFile;
};
