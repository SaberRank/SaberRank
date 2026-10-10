#pragma once

#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <string>

namespace SnoreSaber::Core::Gameplay
{
    enum class SnoreSaberPlayOutcome
    {
        Clear,
        Fail,
        Quit,
        Restart,
    };

    SnoreSaberPlayOutcome FromLevelCompletionResults(GlobalNamespace::LevelCompletionResults* levelCompletionResults);
    std::string ToString(SnoreSaberPlayOutcome playOutcome);
}
