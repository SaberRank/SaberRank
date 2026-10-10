#include "Features/Replays/Playback/ReplayNoteMissEventGuard.hpp"

#include <unordered_set>

namespace
{
    std::unordered_set<GlobalNamespace::NoteController*> allowedReplayMisses;
}

namespace SnoreSaber::ReplaySystem::Playback::ReplayNoteMissEventGuard
{
    void Allow(GlobalNamespace::NoteController* noteController)
    {
        if (noteController)
        {
            allowedReplayMisses.insert(noteController);
        }
    }

    void Clear(GlobalNamespace::NoteController* noteController)
    {
        if (noteController)
        {
            allowedReplayMisses.erase(noteController);
        }
    }

    bool IsAllowed(GlobalNamespace::NoteController* noteController)
    {
        return noteController && allowedReplayMisses.contains(noteController);
    }
}
