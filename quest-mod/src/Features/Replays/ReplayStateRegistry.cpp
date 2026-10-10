#include "Features/Replays/ReplayStateRegistry.hpp"

namespace SnoreSaber::ReplaySystem::ReplayStateRegistry
{
    ReplayState Current;

    bool IsPlaybackEnabled()
    {
        return Current.isPlaybackEnabled;
    }

    bool IsModernPlaybackEnabled()
    {
        return Current.isPlaybackEnabled && !Current.isLegacyReplay;
    }

    bool IsLegacyPlaybackEnabled()
    {
        return Current.isPlaybackEnabled && Current.isLegacyReplay;
    }

    void Use(ReplayState replayState)
    {
        Current.Reset();
        Current.BeginReplay(replayState.currentBeatmapLevel.ptr(),
                            replayState.currentBeatmapKey ? *replayState.currentBeatmapKey : GlobalNamespace::BeatmapKey(),
                            replayState.currentModifiers,
                            replayState.currentPlayerName);
        Current.isLegacyReplay = replayState.isLegacyReplay;
        Current.isPlaybackEnabled = replayState.isPlaybackEnabled;
        Current.loadedReplayFile = replayState.loadedReplayFile;
    }
}
