#pragma once

#include <cstdint>

namespace GlobalNamespace
{
    enum class NoteData_ScoringType : std::int32_t
    {
        Ignore = -1,
        NoScore = 0,
        Normal = 1,
        ArcHead = 2,
        ArcTail = 3,
        ChainHead = 4,
        ChainLink = 5,
        ArcHeadArcTail = 6,
        ChainHeadArcTail = 7,
        ChainLinkArcHead = 8,
    };
} // namespace GlobalNamespace
