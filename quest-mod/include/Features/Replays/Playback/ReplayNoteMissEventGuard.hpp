#pragma once

#include <GlobalNamespace/NoteController.hpp>

namespace SnoreSaber::ReplaySystem::Playback::ReplayNoteMissEventGuard
{
    void Allow(GlobalNamespace::NoteController* noteController);
    void Clear(GlobalNamespace::NoteController* noteController);
    bool IsAllowed(GlobalNamespace::NoteController* noteController);
}
