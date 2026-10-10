#pragma once

#include <GlobalNamespace/NoteData.hpp>
#include <optional>

#include "Utils/Versions.hpp"

namespace SnoreSaber::Data::Private
{
    enum class ScoringType_pre1_40
    {
        Ignore = -1,
        NoScore,
        Normal,
        SliderHead,
        SliderTail,
        BurstSliderHead,
        BurstSliderElement
    };

    enum class ScoringType_1_40
    {
        Ignore = -1,
        NoScore,
        Normal,
        ArcHead,
        ArcTail,
        ChainHead,
        ChainLink,
        ArcHeadArcTail,
        ChainHeadArcTail,
        ChainLinkArcHead,
        ChainHeadArcHead, // 1.40.9+
        ChainHeadArcHeadArcTail
    };

    enum class ScoringTypeEra
    {
        Pre1_40,
        From1_40_0,
        From1_40_9
    };

    // replays keep values from the game that recorded them, so compare by note parts
    namespace ReplayScoringTypes
    {
        ScoringTypeEra EraOf(const std::optional<version>& gameVersion);
        bool Matches(int storedScoringType, ScoringTypeEra storedEra, GlobalNamespace::NoteData_ScoringType gameScoringType);
    }
}
