#include "Features/Replays/ReplayPlaybackContext.hpp"

#include "Features/Replays/ReplayStateRegistry.hpp"

#include <algorithm>

DEFINE_TYPE(SnoreSaber::ReplaySystem, ReplayPlaybackContext);

namespace SnoreSaber::ReplaySystem
{
    void ReplayPlaybackContext::ctor()
    {
        INVOKE_CTOR();
        UseCurrentState();
    }

    void ReplayPlaybackContext::UseCurrentState()
    {
        _beatmapLevel = ReplayStateRegistry::Current.currentBeatmapLevel.ptr();
        if (ReplayStateRegistry::Current.currentBeatmapKey)
        {
            _beatmapKey = *ReplayStateRegistry::Current.currentBeatmapKey;
        }
        else
        {
            _beatmapKey = GlobalNamespace::BeatmapKey();
        }

        _playerName = ReplayStateRegistry::Current.currentPlayerName;
        _modifiers = ReplayStateRegistry::Current.currentModifiers;
        _isLegacyReplay = ReplayStateRegistry::Current.isLegacyReplay;
        _isPlaybackEnabled = ReplayStateRegistry::Current.isPlaybackEnabled;
        _replayFile = ReplayStateRegistry::Current.loadedReplayFile;
    }

    bool ReplayPlaybackContext::IsPlaybackEnabled() const
    {
        return _isPlaybackEnabled;
    }

    bool ReplayPlaybackContext::IsModernPlaybackEnabled() const
    {
        return _isPlaybackEnabled && !_isLegacyReplay && _replayFile;
    }

    std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> ReplayPlaybackContext::GetReplayFile() const
    {
        return _replayFile;
    }

    GlobalNamespace::BeatmapLevel* ReplayPlaybackContext::GetBeatmapLevel() const
    {
        return _beatmapLevel.ptr();
    }

    GlobalNamespace::BeatmapKey ReplayPlaybackContext::GetBeatmapKey() const
    {
        if (_beatmapKey)
        {
            return *_beatmapKey;
        }

        return GlobalNamespace::BeatmapKey();
    }

    const std::u16string& ReplayPlaybackContext::GetPlayerName() const
    {
        return _playerName;
    }

    const std::string& ReplayPlaybackContext::GetModifiers() const
    {
        return _modifiers;
    }

    float ReplayPlaybackContext::GetInitialTimeScale() const
    {
        if (!_replayFile || _replayFile->noteKeyframes.empty())
        {
            return 1.0f;
        }

        return _replayFile->noteKeyframes.front().TimeSyncTimescale;
    }

    bool ReplayPlaybackContext::HasModifier(std::string_view modifier) const
    {
        if (!_replayFile || !_replayFile->metadata)
        {
            return false;
        }

        return std::find(_replayFile->metadata->Modifiers.begin(), _replayFile->metadata->Modifiers.end(), modifier) != _replayFile->metadata->Modifiers.end();
    }
}
