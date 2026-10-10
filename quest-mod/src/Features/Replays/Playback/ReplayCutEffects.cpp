#include "Features/Replays/Playback/ReplayCutEffects.hpp"

#include "Features/Replays/ReplayStateRegistry.hpp"

namespace
{
    bool reduceDebris = false;
}

namespace SnoreSaber::ReplaySystem::Playback::ReplayCutEffects
{
    void SetReduceDebris(bool value)
    {
        reduceDebris = value;
    }

    void Reset()
    {
        reduceDebris = false;
    }

    bool ShouldSuppressDebris()
    {
        return reduceDebris && SnoreSaber::ReplaySystem::ReplayStateRegistry::IsPlaybackEnabled();
    }
}
