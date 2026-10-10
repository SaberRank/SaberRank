#include "Core/Gameplay/SnoreSaberPlayOutcome.hpp"

using namespace GlobalNamespace;

namespace SnoreSaber::Core::Gameplay
{
    SnoreSaberPlayOutcome FromLevelCompletionResults(LevelCompletionResults* levelCompletionResults)
    {
        if (levelCompletionResults->levelEndStateType == LevelCompletionResults::LevelEndStateType::Failed)
        {
            return SnoreSaberPlayOutcome::Fail;
        }

        if (levelCompletionResults->levelEndAction == LevelCompletionResults::LevelEndAction::Restart)
        {
            return SnoreSaberPlayOutcome::Restart;
        }

        if (levelCompletionResults->levelEndAction == LevelCompletionResults::LevelEndAction::Quit)
        {
            return SnoreSaberPlayOutcome::Quit;
        }

        return SnoreSaberPlayOutcome::Clear;
    }

    std::string ToString(SnoreSaberPlayOutcome playOutcome)
    {
        switch (playOutcome)
        {
            case SnoreSaberPlayOutcome::Fail:
                return "FAIL";
            case SnoreSaberPlayOutcome::Quit:
                return "QUIT";
            case SnoreSaberPlayOutcome::Restart:
                return "RESTART";
            case SnoreSaberPlayOutcome::Clear:
            default:
                return "CLEAR";
        }
    }
}
