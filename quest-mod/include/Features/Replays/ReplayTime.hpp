#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include <vector>

namespace SnoreSaber::ReplaySystem::Playback::ReplayTimeSearch
{
    template<typename TimeAt>
    int UpperBound(int count, TimeAt timeAt, float time, bool inclusive)
    {
        int low = 0;
        int high = count;
        while (low < high)
        {
            int mid = low + ((high - low) / 2);
            if (inclusive ? timeAt(mid) <= time : timeAt(mid) < time)
            {
                low = mid + 1;
            }
            else
            {
                high = mid;
            }
        }

        return low;
    }

    template<typename T, typename TimeSelector>
    int CountAtOrBefore(const std::vector<T>& values, float time, TimeSelector timeSelector)
    {
        return UpperBound(static_cast<int>(values.size()), [&](int index) {
            return timeSelector(values[index]);
        }, time, true);
    }

    inline int CountAtOrBefore(const std::vector<float>& times, float time)
    {
        return UpperBound(static_cast<int>(times.size()), [&](int index) {
            return times[index];
        }, time, true);
    }

    inline int CountBefore(const std::vector<float>& times, float time)
    {
        return UpperBound(static_cast<int>(times.size()), [&](int index) {
            return times[index];
        }, time, false);
    }

    inline bool IsScoringNoteEvent(const SnoreSaber::Data::Private::NoteEvent& noteEvent)
    {
        auto eventType = noteEvent.EventType;
        return eventType == SnoreSaber::Data::Private::NoteEventType::GoodCut ||
               eventType == SnoreSaber::Data::Private::NoteEventType::BadCut ||
               eventType == SnoreSaber::Data::Private::NoteEventType::Miss;
    }
}
