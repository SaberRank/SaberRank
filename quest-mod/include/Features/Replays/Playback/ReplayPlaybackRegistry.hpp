#pragma once

#include "Features/Replays/Playback/NotePlayer.hpp"
#include "Utils/SafePtr.hpp"

namespace SnoreSaber::ReplaySystem::Playback::ReplayPlaybackRegistry
{
    void SetNotePlayer(NotePlayer* notePlayer);
    NotePlayer* GetNotePlayer();
    void Clear();
}
