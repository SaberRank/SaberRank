#include "Features/Replays/Format/ReplayScoringTypes.hpp"

using namespace GlobalNamespace;

namespace SnoreSaber::Data::Private
{
    namespace
    {
        namespace RelevantGameVersions
        {
            const auto Version_1_40 = version("1.40.0");
            const auto Version_1_40_9 = version("1.40.9");
        }

        constexpr auto GameEra = ScoringTypeEra::From1_40_0; // quest targets 1.40.8; bucket ends at 1.40.8

        enum NoteParts
        {
            None = 0,
            ArcHead = 1,
            ArcTail = 2,
            ChainHead = 4,
            ChainLink = 8
        };

        NoteParts LegacyParts(ScoringType_pre1_40 scoringType)
        {
            switch (scoringType)
            {
                case ScoringType_pre1_40::SliderHead:
                    return NoteParts::ArcHead;
                case ScoringType_pre1_40::SliderTail:
                    return NoteParts::ArcTail;
                case ScoringType_pre1_40::BurstSliderHead:
                    return NoteParts::ChainHead;
                case ScoringType_pre1_40::BurstSliderElement:
                    return NoteParts::ChainLink;
                default:
                    return NoteParts::None;
            }
        }

        NoteParts ModernParts(ScoringType_1_40 scoringType)
        {
            switch (scoringType)
            {
                case ScoringType_1_40::ArcHead:
                    return NoteParts::ArcHead;
                case ScoringType_1_40::ArcTail:
                    return NoteParts::ArcTail;
                case ScoringType_1_40::ChainHead:
                    return NoteParts::ChainHead;
                case ScoringType_1_40::ChainLink:
                    return NoteParts::ChainLink;
                case ScoringType_1_40::ArcHeadArcTail:
                    return static_cast<NoteParts>(NoteParts::ArcHead | NoteParts::ArcTail);
                case ScoringType_1_40::ChainHeadArcTail:
                    return static_cast<NoteParts>(NoteParts::ChainHead | NoteParts::ArcTail);
                case ScoringType_1_40::ChainLinkArcHead:
                    return static_cast<NoteParts>(NoteParts::ChainLink | NoteParts::ArcHead);
                case ScoringType_1_40::ChainHeadArcHead:
                    return static_cast<NoteParts>(NoteParts::ChainHead | NoteParts::ArcHead);
                case ScoringType_1_40::ChainHeadArcHeadArcTail:
                    return static_cast<NoteParts>(NoteParts::ChainHead | NoteParts::ArcHead | NoteParts::ArcTail);
                default:
                    return NoteParts::None;
            }
        }

        NoteParts Parts(int scoringType, ScoringTypeEra era)
        {
            return era == ScoringTypeEra::Pre1_40 ? LegacyParts(static_cast<ScoringType_pre1_40>(scoringType)) : ModernParts(static_cast<ScoringType_1_40>(scoringType));
        }
    }

    ScoringTypeEra ReplayScoringTypes::EraOf(const std::optional<version>& gameVersion)
    {
        if (!gameVersion || *gameVersion < RelevantGameVersions::Version_1_40)
        {
            return ScoringTypeEra::Pre1_40;
        }
        return *gameVersion < RelevantGameVersions::Version_1_40_9 ? ScoringTypeEra::From1_40_0 : ScoringTypeEra::From1_40_9;
    }

    bool ReplayScoringTypes::Matches(int storedScoringType, ScoringTypeEra storedEra, NoteData_ScoringType gameScoringType)
    {
        if (storedEra == GameEra)
        {
            return storedScoringType == (int)gameScoringType;
        }

        NoteParts storedParts = Parts(storedScoringType, storedEra);
        NoteParts gameParts = Parts((int)gameScoringType, GameEra);
        // partless values line up in every era
        return storedParts == NoteParts::None || gameParts == NoteParts::None
            ? storedScoringType == (int)gameScoringType
            : (storedParts & gameParts) != 0;
    }
}
