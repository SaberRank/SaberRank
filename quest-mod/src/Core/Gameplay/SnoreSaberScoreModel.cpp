#include "Core/Gameplay/SnoreSaberScoreModel.hpp"

namespace SnoreSaber::Core::Gameplay::SnoreSaberScoreModel
{
    int OldMaxRawScoreForNumberOfNotes(int noteCount)
    {
        int score = 0;
        int multiplier = 1;
        while (multiplier < 8)
        {
            if (noteCount >= multiplier * 2)
            {
                score += multiplier * multiplier * 2 + multiplier;
                noteCount -= multiplier * 2;
                multiplier *= 2;
                continue;
            }

            score += multiplier * noteCount;
            noteCount = 0;
            break;
        }

        score += noteCount * multiplier;
        return score * 115;
    }
}
