#include "Features/Replays/Playback/ReplayPlaybackRegistry.hpp"

namespace SnoreSaber::ReplaySystem::Playback::ReplayPlaybackRegistry
{
    namespace
    {
        SafePtr<NotePlayer> notePlayerInstance;
    }

    void SetNotePlayer(NotePlayer* notePlayer)
    {
        notePlayerInstance = notePlayer;
    }

    NotePlayer* GetNotePlayer()
    {
        return notePlayerInstance.ptr();
    }

    void Clear()
    {
        notePlayerInstance = nullptr;
    }
}
