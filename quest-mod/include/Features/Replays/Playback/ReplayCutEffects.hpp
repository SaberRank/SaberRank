#pragma once

namespace SnoreSaber::ReplaySystem::Playback::ReplayCutEffects
{
    void SetReduceDebris(bool value);
    void Reset();
    bool ShouldSuppressDebris();
}
