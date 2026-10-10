#pragma once

#include "Features/Replays/ReplayState.hpp"

namespace SnoreSaber::ReplaySystem::ReplayStateRegistry
{
    extern ReplayState Current;

    bool IsPlaybackEnabled();
    bool IsModernPlaybackEnabled();
    bool IsLegacyPlaybackEnabled();
    void Use(ReplayState replayState);
}
