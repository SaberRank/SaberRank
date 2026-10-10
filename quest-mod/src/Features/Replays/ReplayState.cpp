#include "Features/Replays/ReplayState.hpp"

#include "Features/Replays/Playback/CutSoundEffectBuffer.hpp"
#include "Features/Replays/Playback/ReplayCutEffects.hpp"

namespace SnoreSaber::ReplaySystem
{
    void ReplayState::Reset()
    {
        Playback::CutSoundEffectBuffer::Reset();
        Playback::ReplayCutEffects::Reset();
        currentBeatmapLevel = nullptr;
        currentBeatmapKey = GlobalNamespace::BeatmapKey();
        currentModifiers.clear();
        currentPlayerName.clear();
        isLegacyReplay = false;
        isPlaybackEnabled = false;
        loadedReplayFile = nullptr;
    }

    void ReplayState::BeginReplay(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, std::string modifiers, std::u16string playerName)
    {
        Playback::CutSoundEffectBuffer::Reset();
        Playback::ReplayCutEffects::Reset();
        currentBeatmapLevel = beatmapLevel;
        currentBeatmapKey = beatmapKey;
        currentModifiers = modifiers;
        currentPlayerName = playerName;
    }

    void ReplayState::LoadReplay(std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> replay)
    {
        loadedReplayFile = std::move(replay);
        isLegacyReplay = false;
        isPlaybackEnabled = static_cast<bool>(loadedReplayFile);
    }

    void ReplayState::EndPlayback()
    {
        Playback::CutSoundEffectBuffer::Reset();
        Playback::ReplayCutEffects::Reset();
        isPlaybackEnabled = false;
    }
}
