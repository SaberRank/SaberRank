#include "Features/Replays/Format/HsvReplayConfigCodec.hpp"

#include <beatsaber-hook/shared/rapidjson/include/rapidjson/document.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <set>
#include <stdexcept>

// 1:1 port of the PC HsvReplayConfigCodec: same accepted configs, same binary payload.
// parse phase mirrors Newtonsoft semantics (case-insensitive members, null == missing),
// normalize phase reproduces the PC failure strings and ordering checks.
namespace
{
    using rapidjson::Value;

    constexpr uint64_t LatestSupportedMajor = 3;
    constexpr uint64_t LatestSupportedMinor = 7;
    constexpr int MaxListItems = 32;
    constexpr int MaxStringBytes = 512;

    constexpr uint8_t FlagFixedPosition = 1 << 0;
    constexpr uint8_t FlagTargetPositionOffset = 1 << 1;
    constexpr uint8_t FlagDoIntermediateUpdates = 1 << 2;
    constexpr uint8_t FlagAssumeMaxPostSwing = 1 << 3;
    constexpr uint8_t FlagChainLinkDisplay = 1 << 4;
    constexpr uint8_t FlagRandomizeBadCutDisplays = 1 << 5;
    constexpr uint8_t FlagRandomizeMissDisplays = 1 << 6;

    // HsvDisplayMode None/Format/TextOnly/Numeric/ScoreOnTop/Directions = 0..5
    constexpr std::string_view DisplayModeNames[] = {"None", "Format", "TextOnly", "Numeric", "ScoreOnTop", "Directions"};
    constexpr int DisplayModeNumeric = 3;
    // HsvBadCutDisplayType All/WrongDirection/WrongColor/Bomb = 0..3
    constexpr std::string_view BadCutDisplayTypeNames[] = {"All", "WrongDirection", "WrongColor", "Bomb"};
    constexpr int BadCutDisplayTypeAll = 0;

    struct ParseFailure : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct HsvColor
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
    };

    struct HsvVector3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct JudgmentJson
    {
        int threshold = 0;
        std::string text;
        std::optional<HsvColor> color;
        bool fade = false;
    };

    struct ColoredTextJson
    {
        std::string text;
        std::optional<HsvColor> color;
    };

    struct BadCutDisplayJson
    {
        std::string text;
        std::optional<HsvColor> color;
        std::optional<int> type;
    };

    struct SegmentJson
    {
        int threshold = 0;
        std::string text;
    };

    struct TimeSegmentJson
    {
        float threshold = 0.0f;
        std::string text;
    };

    // list elements stay optional so "contain an empty entry" checks match PC ordering
    struct ConfigJson
    {
        uint64_t majorVersion = 0;
        uint64_t minorVersion = 0;
        uint64_t patchVersion = 0;
        int displayMode = DisplayModeNumeric;
        std::optional<HsvVector3> fixedPosition;
        std::optional<HsvVector3> targetPositionOffset;
        bool doIntermediateUpdates = true;
        bool assumeMaxPostSwing = false;
        std::vector<std::optional<JudgmentJson>> judgments;
        std::vector<std::optional<JudgmentJson>> chainHeadJudgments;
        std::optional<ColoredTextJson> chainLinkDisplay;
        std::vector<std::optional<SegmentJson>> beforeCutAngleJudgments;
        std::vector<std::optional<SegmentJson>> accuracyJudgments;
        std::vector<std::optional<SegmentJson>> afterCutAngleJudgments;
        int timeDependenceDecimalPrecision = 1;
        int timeDependenceDecimalOffset = 2;
        std::vector<std::optional<TimeSegmentJson>> timeDependenceJudgments;
        bool randomizeBadCutDisplays = true;
        std::vector<std::optional<BadCutDisplayJson>> badCutDisplays;
        bool randomizeMissDisplays = true;
        std::vector<std::optional<ColoredTextJson>> missDisplays;
    };

    struct Judgment
    {
        int threshold;
        std::string text;
        HsvColor color;
        bool fade;
    };

    struct ColoredText
    {
        std::string text;
        HsvColor color;
    };

    struct BadCutDisplay
    {
        std::string text;
        HsvColor color;
        uint8_t type;
    };

    struct Segment
    {
        int threshold;
        std::string text;
    };

    struct TimeSegment
    {
        float threshold;
        std::string text;
    };

    struct HsvReplayConfig
    {
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint8_t patchVersion;
        uint8_t displayMode;
        std::optional<HsvVector3> fixedPosition;
        std::optional<HsvVector3> targetPositionOffset;
        bool doIntermediateUpdates;
        bool assumeMaxPostSwing;
        std::vector<Judgment> judgments;
        std::vector<Judgment> chainHeadJudgments;
        std::optional<ColoredText> chainLinkDisplay;
        std::vector<Segment> beforeCutAngleJudgments;
        std::vector<Segment> accuracyJudgments;
        std::vector<Segment> afterCutAngleJudgments;
        int timeDependenceDecimalPrecision;
        int timeDependenceDecimalOffset;
        std::vector<TimeSegment> timeDependenceJudgments;
        bool randomizeBadCutDisplays;
        std::vector<BadCutDisplay> badCutDisplays;
        bool randomizeMissDisplays;
        std::vector<ColoredText> missDisplays;
    };

    bool EqualsIgnoreCase(std::string_view left, std::string_view right)
    {
        if (left.size() != right.size())
        {
            return false;
        }
        for (size_t index = 0; index < left.size(); index++)
        {
            if (std::tolower((unsigned char)left[index]) != std::tolower((unsigned char)right[index]))
            {
                return false;
            }
        }
        return true;
    }

    // Newtonsoft member resolution: exact match first, then case-insensitive; null tokens count as missing
    const Value* FindMember(const Value& object, std::string_view name)
    {
        const Value* found = nullptr;
        for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it)
        {
            std::string_view memberName(it->name.GetString(), it->name.GetStringLength());
            if (memberName == name)
            {
                found = &it->value;
                break;
            }
            if (!found && EqualsIgnoreCase(memberName, name))
            {
                found = &it->value;
            }
        }
        if (found && found->IsNull())
        {
            return nullptr;
        }
        return found;
    }

    uint64_t GetVersion(const Value& object, std::string_view name)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return 0;
        }
        if (!member->IsUint64())
        {
            throw ParseFailure(std::string(name) + " is not a valid version number");
        }
        return member->GetUint64();
    }

    bool GetBool(const Value& object, std::string_view name, bool defaultValue)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return defaultValue;
        }
        if (!member->IsBool())
        {
            throw ParseFailure(std::string(name) + " is not a boolean");
        }
        return member->GetBool();
    }

    int GetInt(const Value& object, std::string_view name, int defaultValue)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return defaultValue;
        }
        if (!member->IsInt())
        {
            throw ParseFailure(std::string(name) + " is not an integer");
        }
        return member->GetInt();
    }

    float GetFloat(const Value& object, std::string_view name, float defaultValue)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return defaultValue;
        }
        if (!member->IsNumber())
        {
            throw ParseFailure(std::string(name) + " is not a number");
        }
        return (float)member->GetDouble();
    }

    std::string GetString(const Value& object, std::string_view name)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return std::string();
        }
        if (!member->IsString())
        {
            throw ParseFailure(std::string(name) + " is not a string");
        }
        return std::string(member->GetString(), member->GetStringLength());
    }

    float NumberAt(const Value& array, int index, std::string_view what)
    {
        const Value& element = array[index];
        if (!element.IsNumber())
        {
            throw ParseFailure(std::string("Invalid HSV ") + std::string(what));
        }
        return (float)element.GetDouble();
    }

    const Value* ExactMember(const Value& object, std::string_view name)
    {
        for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it)
        {
            std::string_view memberName(it->name.GetString(), it->name.GetStringLength());
            if (memberName == name && !it->value.IsNull())
            {
                return &it->value;
            }
        }
        return nullptr;
    }

    // JObject indexers on PC are case sensitive, hence the explicit lower/upper fallbacks
    float ObjectComponent(const Value& object, std::string_view lower, std::string_view upper, float defaultValue, std::string_view what)
    {
        for (std::string_view name : {lower, upper})
        {
            const Value* member = ExactMember(object, name);
            if (member)
            {
                if (!member->IsNumber())
                {
                    throw ParseFailure(std::string("Invalid HSV ") + std::string(what));
                }
                return (float)member->GetDouble();
            }
        }
        return defaultValue;
    }

    HsvColor ParseColor(const Value& value)
    {
        if (value.IsArray() && value.Size() >= 4)
        {
            return HsvColor {NumberAt(value, 0, "color"), NumberAt(value, 1, "color"), NumberAt(value, 2, "color"), NumberAt(value, 3, "color")};
        }
        if (value.IsObject())
        {
            return HsvColor {
                    ObjectComponent(value, "r", "R", 1.0f, "color"),
                    ObjectComponent(value, "g", "G", 1.0f, "color"),
                    ObjectComponent(value, "b", "B", 1.0f, "color"),
                    ObjectComponent(value, "a", "A", 1.0f, "color"),
            };
        }
        throw ParseFailure("Invalid HSV color");
    }

    std::optional<HsvColor> GetColor(const Value& object, std::string_view name)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return std::nullopt;
        }
        return ParseColor(*member);
    }

    HsvVector3 ParseVector(const Value& value)
    {
        if (value.IsObject())
        {
            return HsvVector3 {
                    ObjectComponent(value, "x", "X", 0.0f, "vector"),
                    ObjectComponent(value, "y", "Y", 0.0f, "vector"),
                    ObjectComponent(value, "z", "Z", 0.0f, "vector"),
            };
        }
        if (value.IsArray() && value.Size() >= 3)
        {
            return HsvVector3 {NumberAt(value, 0, "vector"), NumberAt(value, 1, "vector"), NumberAt(value, 2, "vector")};
        }
        throw ParseFailure("Invalid HSV vector");
    }

    std::optional<HsvVector3> GetVector(const Value& object, std::string_view name)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return std::nullopt;
        }
        return ParseVector(*member);
    }

    // StringEnumConverter: names case-insensitively, integers within the byte-backed enum range
    int GetEnum(const Value& object, std::string_view name, std::string_view const* names, int nameCount, int defaultValue)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return defaultValue;
        }
        if (member->IsInt())
        {
            int value = member->GetInt();
            if (value < 0 || value > 255)
            {
                throw ParseFailure(std::string(name) + " is out of range");
            }
            return value;
        }
        if (member->IsString())
        {
            std::string_view value(member->GetString(), member->GetStringLength());
            for (int index = 0; index < nameCount; index++)
            {
                if (EqualsIgnoreCase(value, names[index]))
                {
                    return index;
                }
            }
        }
        throw ParseFailure(std::string(name) + " is not a valid enum value");
    }

    const Value* GetArray(const Value& object, std::string_view name)
    {
        const Value* member = FindMember(object, name);
        if (!member)
        {
            return nullptr;
        }
        if (!member->IsArray())
        {
            throw ParseFailure(std::string(name) + " is not an array");
        }
        return member;
    }

    template <typename T, typename Parser>
    std::vector<std::optional<T>> ParseList(const Value& object, std::string_view name, Parser parser)
    {
        std::vector<std::optional<T>> values;
        const Value* array = GetArray(object, name);
        if (!array)
        {
            return values;
        }
        values.reserve(array->Size());
        for (rapidjson::SizeType index = 0; index < array->Size(); index++)
        {
            const Value& element = (*array)[index];
            if (element.IsNull())
            {
                values.push_back(std::nullopt);
                continue;
            }
            if (!element.IsObject())
            {
                throw ParseFailure(std::string(name) + " contains an invalid entry");
            }
            values.push_back(parser(element));
        }
        return values;
    }

    JudgmentJson ParseJudgment(const Value& value)
    {
        return JudgmentJson {GetInt(value, "threshold", 0), GetString(value, "text"), GetColor(value, "color"), GetBool(value, "fade", false)};
    }

    ColoredTextJson ParseColoredText(const Value& value)
    {
        return ColoredTextJson {GetString(value, "text"), GetColor(value, "color")};
    }

    BadCutDisplayJson ParseBadCutDisplay(const Value& value)
    {
        BadCutDisplayJson display {GetString(value, "text"), GetColor(value, "color"), std::nullopt};
        if (FindMember(value, "type"))
        {
            display.type = GetEnum(value, "type", BadCutDisplayTypeNames, 4, BadCutDisplayTypeAll);
        }
        return display;
    }

    SegmentJson ParseSegment(const Value& value)
    {
        return SegmentJson {GetInt(value, "threshold", 0), GetString(value, "text")};
    }

    TimeSegmentJson ParseTimeSegment(const Value& value)
    {
        return TimeSegmentJson {GetFloat(value, "threshold", 0.0f), GetString(value, "text")};
    }

    ConfigJson ParseConfig(std::string_view json)
    {
        rapidjson::Document document;
        document.Parse(json.data(), json.size());
        if (document.HasParseError() || !document.IsObject())
        {
            throw ParseFailure("invalid JSON document");
        }

        ConfigJson config;
        config.majorVersion = GetVersion(document, "majorVersion");
        config.minorVersion = GetVersion(document, "minorVersion");
        config.patchVersion = GetVersion(document, "patchVersion");
        config.displayMode = GetEnum(document, "displayMode", DisplayModeNames, 6, DisplayModeNumeric);
        config.fixedPosition = GetVector(document, "fixedPosition");
        config.targetPositionOffset = GetVector(document, "targetPositionOffset");
        config.doIntermediateUpdates = GetBool(document, "doIntermediateUpdates", true);
        config.assumeMaxPostSwing = GetBool(document, "assumeMaxPostSwing", false);
        config.judgments = ParseList<JudgmentJson>(document, "judgments", ParseJudgment);
        config.chainHeadJudgments = ParseList<JudgmentJson>(document, "chainHeadJudgments", ParseJudgment);
        const Value* chainLinkDisplay = FindMember(document, "chainLinkDisplay");
        if (chainLinkDisplay)
        {
            if (!chainLinkDisplay->IsObject())
            {
                throw ParseFailure("chainLinkDisplay is not an object");
            }
            config.chainLinkDisplay = ParseColoredText(*chainLinkDisplay);
        }
        config.beforeCutAngleJudgments = ParseList<SegmentJson>(document, "beforeCutAngleJudgments", ParseSegment);
        config.accuracyJudgments = ParseList<SegmentJson>(document, "accuracyJudgments", ParseSegment);
        config.afterCutAngleJudgments = ParseList<SegmentJson>(document, "afterCutAngleJudgments", ParseSegment);
        config.timeDependenceDecimalPrecision = GetInt(document, "timeDependenceDecimalPrecision", 1);
        config.timeDependenceDecimalOffset = GetInt(document, "timeDependenceDecimalOffset", 2);
        config.timeDependenceJudgments = ParseList<TimeSegmentJson>(document, "timeDependenceJudgments", ParseTimeSegment);
        config.randomizeBadCutDisplays = GetBool(document, "randomizeBadCutDisplays", true);
        config.badCutDisplays = ParseList<BadCutDisplayJson>(document, "badCutDisplays", ParseBadCutDisplay);
        config.randomizeMissDisplays = GetBool(document, "randomizeMissDisplays", true);
        config.missDisplays = ParseList<ColoredTextJson>(document, "missDisplays", ParseColoredText);
        return config;
    }

    bool ValidateString(const std::string& value, const std::string& name, std::string& failure)
    {
        if (value.size() > MaxStringBytes)
        {
            failure = "HSV " + name + " contain text that is too long";
            return false;
        }
        return true;
    }

    bool ValidateThresholds(const std::vector<int>& thresholds, const std::string& name, std::string& failure)
    {
        std::optional<int> previous;
        std::set<int> seen;
        for (int threshold : thresholds)
        {
            if (threshold < 0 || threshold > 65535)
            {
                failure = "HSV " + name + " contain an out of range threshold";
                return false;
            }
            if (previous.has_value() && threshold > previous.value())
            {
                failure = "HSV " + name + " are not descending";
                return false;
            }
            if (!seen.insert(threshold).second)
            {
                failure = "HSV " + name + " contain duplicate thresholds";
                return false;
            }
            previous = threshold;
        }
        return true;
    }

    template <typename T>
    bool CheckListShape(const std::vector<std::optional<T>>& source, const std::string& name, std::string& failure)
    {
        if (source.size() > MaxListItems)
        {
            failure = "HSV " + name + " contain too many entries";
            return false;
        }
        for (const auto& value : source)
        {
            if (!value.has_value())
            {
                failure = "HSV " + name + " contain an empty entry";
                return false;
            }
        }
        return true;
    }

    bool NormalizeJudgments(const std::vector<std::optional<JudgmentJson>>& source, const std::string& name, std::vector<Judgment>& values, std::string& failure)
    {
        if (source.empty())
        {
            failure = "HSV " + name + " are empty";
            return false;
        }
        if (!CheckListShape(source, name, failure))
        {
            return false;
        }

        values.clear();
        values.reserve(source.size());
        for (const auto& value : source)
        {
            values.push_back(Judgment {value->threshold, value->text, value->color.value_or(HsvColor {}), value->fade});
        }
        std::stable_sort(values.begin(), values.end(), [](const Judgment& left, const Judgment& right) { return left.threshold > right.threshold; });

        if (values[0].fade)
        {
            failure = "first HSV " + name + " entry cannot fade";
            return false;
        }

        std::vector<int> thresholds;
        thresholds.reserve(values.size());
        for (const Judgment& value : values)
        {
            thresholds.push_back(value.threshold);
        }
        if (!ValidateThresholds(thresholds, name, failure))
        {
            return false;
        }

        for (const Judgment& value : values)
        {
            if (!ValidateString(value.text, name, failure))
            {
                return false;
            }
        }
        return true;
    }

    bool NormalizeColoredText(const std::optional<ColoredTextJson>& source, const std::string& name, std::optional<ColoredText>& value, std::string& failure)
    {
        value = std::nullopt;
        if (!source.has_value())
        {
            return true;
        }
        if (!ValidateString(source->text, name, failure))
        {
            return false;
        }
        value = ColoredText {source->text, source->color.value_or(HsvColor {})};
        return true;
    }

    bool NormalizeSegments(const std::vector<std::optional<SegmentJson>>& source, const std::string& name, std::vector<Segment>& values, std::string& failure)
    {
        values.clear();
        if (source.empty())
        {
            return true;
        }
        if (!CheckListShape(source, name, failure))
        {
            return false;
        }

        values.reserve(source.size());
        for (const auto& value : source)
        {
            values.push_back(Segment {value->threshold, value->text});
        }
        std::stable_sort(values.begin(), values.end(), [](const Segment& left, const Segment& right) { return left.threshold > right.threshold; });

        std::vector<int> thresholds;
        thresholds.reserve(values.size());
        for (const Segment& value : values)
        {
            thresholds.push_back(value.threshold);
        }
        if (!ValidateThresholds(thresholds, name, failure))
        {
            return false;
        }

        for (const Segment& value : values)
        {
            if (!ValidateString(value.text, name, failure))
            {
                return false;
            }
        }
        return true;
    }

    bool NormalizeTimeSegments(const std::vector<std::optional<TimeSegmentJson>>& source, const std::string& name, std::vector<TimeSegment>& values, std::string& failure)
    {
        values.clear();
        if (source.empty())
        {
            return true;
        }
        if (!CheckListShape(source, name, failure))
        {
            return false;
        }

        values.reserve(source.size());
        for (const auto& value : source)
        {
            values.push_back(TimeSegment {value->threshold, value->text});
        }
        std::stable_sort(values.begin(), values.end(), [](const TimeSegment& left, const TimeSegment& right) { return left.threshold > right.threshold; });

        for (const TimeSegment& value : values)
        {
            if (std::isnan(value.threshold) || std::isinf(value.threshold))
            {
                failure = "HSV " + name + " contain a non-finite threshold";
                return false;
            }
        }
        for (size_t index = 1; index < values.size(); index++)
        {
            if (values[index - 1].threshold == values[index].threshold)
            {
                failure = "HSV " + name + " contain duplicate thresholds";
                return false;
            }
        }
        for (const TimeSegment& value : values)
        {
            if (!ValidateString(value.text, name, failure))
            {
                return false;
            }
        }
        return true;
    }

    bool NormalizeBadCutDisplays(const std::vector<std::optional<BadCutDisplayJson>>& source, std::vector<BadCutDisplay>& values, std::string& failure)
    {
        values.clear();
        if (source.empty())
        {
            return true;
        }
        if (!CheckListShape(source, "bad cut displays", failure))
        {
            return false;
        }

        values.reserve(source.size());
        for (const auto& value : source)
        {
            if (!ValidateString(value->text, "bad cut displays", failure))
            {
                return false;
            }
            int type = value->type.has_value() && value->type.value() >= 0 && value->type.value() <= 3 ? value->type.value() : BadCutDisplayTypeAll;
            values.push_back(BadCutDisplay {value->text, value->color.value_or(HsvColor {}), (uint8_t)type});
        }
        return true;
    }

    bool NormalizeMissDisplays(const std::vector<std::optional<ColoredTextJson>>& source, std::vector<ColoredText>& values, std::string& failure)
    {
        values.clear();
        if (source.empty())
        {
            return true;
        }
        if (!CheckListShape(source, "miss displays", failure))
        {
            return false;
        }

        values.reserve(source.size());
        for (const auto& value : source)
        {
            std::optional<ColoredText> text;
            if (!NormalizeColoredText(value, "miss displays", text, failure))
            {
                return false;
            }
            values.push_back(text.value());
        }
        return true;
    }

    std::vector<std::optional<JudgmentJson>> DefaultChainHeadJudgments()
    {
        return {JudgmentJson {0, "%s", std::nullopt, false}};
    }

    uint8_t ToVersionByte(uint64_t value)
    {
        return value > 255 ? (uint8_t)255 : (uint8_t)value;
    }

    bool TryNormalize(const ConfigJson& input, HsvReplayConfig& config, std::string& failure)
    {
        if (input.majorVersion != LatestSupportedMajor || input.minorVersion > LatestSupportedMinor)
        {
            failure = "unsupported HSV config version";
            return false;
        }

        if (input.displayMode < 0 || input.displayMode > 5)
        {
            failure = "unsupported HSV display mode";
            return false;
        }

        if (!NormalizeJudgments(input.judgments, "judgments", config.judgments, failure))
        {
            return false;
        }
        if (input.chainHeadJudgments.empty())
        {
            if (!NormalizeJudgments(DefaultChainHeadJudgments(), "chain head judgments", config.chainHeadJudgments, failure))
            {
                return false;
            }
        }
        else if (!NormalizeJudgments(input.chainHeadJudgments, "chain head judgments", config.chainHeadJudgments, failure))
        {
            return false;
        }
        if (!NormalizeColoredText(input.chainLinkDisplay, "chain link display", config.chainLinkDisplay, failure))
        {
            return false;
        }
        if (!NormalizeSegments(input.beforeCutAngleJudgments, "before cut angle judgments", config.beforeCutAngleJudgments, failure))
        {
            return false;
        }
        if (!NormalizeSegments(input.accuracyJudgments, "accuracy judgments", config.accuracyJudgments, failure))
        {
            return false;
        }
        if (!NormalizeSegments(input.afterCutAngleJudgments, "after cut angle judgments", config.afterCutAngleJudgments, failure))
        {
            return false;
        }
        if (!NormalizeTimeSegments(input.timeDependenceJudgments, "time dependence judgments", config.timeDependenceJudgments, failure))
        {
            return false;
        }
        if (!NormalizeBadCutDisplays(input.badCutDisplays, config.badCutDisplays, failure))
        {
            return false;
        }
        if (!NormalizeMissDisplays(input.missDisplays, config.missDisplays, failure))
        {
            return false;
        }

        config.majorVersion = ToVersionByte(input.majorVersion);
        config.minorVersion = ToVersionByte(input.minorVersion);
        config.patchVersion = ToVersionByte(input.patchVersion);
        config.displayMode = (uint8_t)input.displayMode;
        config.fixedPosition = input.fixedPosition;
        config.targetPositionOffset = input.targetPositionOffset;
        config.doIntermediateUpdates = input.doIntermediateUpdates;
        config.assumeMaxPostSwing = input.assumeMaxPostSwing;
        config.timeDependenceDecimalPrecision = input.timeDependenceDecimalPrecision;
        config.timeDependenceDecimalOffset = input.timeDependenceDecimalOffset;
        config.randomizeBadCutDisplays = input.randomizeBadCutDisplays;
        config.randomizeMissDisplays = input.randomizeMissDisplays;

        if (config.timeDependenceDecimalPrecision < 0 || config.timeDependenceDecimalPrecision > 99)
        {
            failure = "HSV time dependence decimal precision is out of range";
            return false;
        }
        if (config.timeDependenceDecimalOffset < 0 || config.timeDependenceDecimalOffset > 38)
        {
            failure = "HSV time dependence decimal offset is out of range";
            return false;
        }

        return true;
    }

    void WriteByte(uint8_t value, std::vector<char>& stream)
    {
        stream.push_back((char)value);
    }

    void WriteUShort(int value, std::vector<char>& stream)
    {
        stream.push_back((char)(uint8_t)value);
        stream.push_back((char)(uint8_t)(value >> 8));
    }

    void WriteFloat(float value, std::vector<char>& stream)
    {
        char bytes[sizeof(float)];
        std::memcpy(bytes, &value, sizeof(float));
        stream.insert(stream.end(), bytes, bytes + sizeof(float));
    }

    void WriteString(const std::string& value, std::vector<char>& stream)
    {
        WriteUShort((int)value.size(), stream);
        stream.insert(stream.end(), value.begin(), value.end());
    }

    uint8_t ToColorByte(float value)
    {
        if (std::isnan(value) || std::isinf(value))
        {
            return 255;
        }
        value = std::max(0.0f, std::min(1.0f, value));
        // banker's rounding matches C# Math.Round
        return (uint8_t)std::nearbyint(value * 255.0f);
    }

    void WriteColor(const HsvColor& value, std::vector<char>& stream)
    {
        WriteByte(ToColorByte(value.r), stream);
        WriteByte(ToColorByte(value.g), stream);
        WriteByte(ToColorByte(value.b), stream);
        WriteByte(ToColorByte(value.a), stream);
    }

    void WriteVector(const HsvVector3& value, std::vector<char>& stream)
    {
        WriteFloat(value.x, stream);
        WriteFloat(value.y, stream);
        WriteFloat(value.z, stream);
    }

    void WriteColoredText(const ColoredText& value, std::vector<char>& stream)
    {
        WriteString(value.text, stream);
        WriteColor(value.color, stream);
    }

    void WriteJudgments(const std::vector<Judgment>& values, std::vector<char>& stream)
    {
        WriteByte((uint8_t)values.size(), stream);
        for (const Judgment& value : values)
        {
            WriteUShort(value.threshold, stream);
            WriteString(value.text, stream);
            WriteColor(value.color, stream);
            WriteByte(value.fade ? 1 : 0, stream);
        }
    }

    void WriteSegments(const std::vector<Segment>& values, std::vector<char>& stream)
    {
        WriteByte((uint8_t)values.size(), stream);
        for (const Segment& value : values)
        {
            WriteUShort(value.threshold, stream);
            WriteString(value.text, stream);
        }
    }

    void WriteTimeSegments(const std::vector<TimeSegment>& values, std::vector<char>& stream)
    {
        WriteByte((uint8_t)values.size(), stream);
        for (const TimeSegment& value : values)
        {
            WriteFloat(value.threshold, stream);
            WriteString(value.text, stream);
        }
    }

    void WriteBadCutDisplays(const std::vector<BadCutDisplay>& values, std::vector<char>& stream)
    {
        WriteByte((uint8_t)values.size(), stream);
        for (const BadCutDisplay& value : values)
        {
            WriteString(value.text, stream);
            WriteColor(value.color, stream);
            WriteByte(value.type, stream);
        }
    }

    void WriteColoredTexts(const std::vector<ColoredText>& values, std::vector<char>& stream)
    {
        WriteByte((uint8_t)values.size(), stream);
        for (const ColoredText& value : values)
        {
            WriteColoredText(value, stream);
        }
    }

    std::vector<char> Encode(const HsvReplayConfig& config)
    {
        std::vector<char> stream;
        WriteByte(config.majorVersion, stream);
        WriteByte(config.minorVersion, stream);
        WriteByte(config.patchVersion, stream);
        WriteByte(config.displayMode, stream);

        uint8_t flags = 0;
        if (config.fixedPosition.has_value()) flags |= FlagFixedPosition;
        if (config.targetPositionOffset.has_value()) flags |= FlagTargetPositionOffset;
        if (config.doIntermediateUpdates) flags |= FlagDoIntermediateUpdates;
        if (config.assumeMaxPostSwing) flags |= FlagAssumeMaxPostSwing;
        if (config.chainLinkDisplay.has_value()) flags |= FlagChainLinkDisplay;
        if (config.randomizeBadCutDisplays) flags |= FlagRandomizeBadCutDisplays;
        if (config.randomizeMissDisplays) flags |= FlagRandomizeMissDisplays;
        WriteByte(flags, stream);

        if (config.fixedPosition.has_value()) WriteVector(config.fixedPosition.value(), stream);
        if (config.targetPositionOffset.has_value()) WriteVector(config.targetPositionOffset.value(), stream);

        WriteByte((uint8_t)config.timeDependenceDecimalPrecision, stream);
        WriteByte((uint8_t)config.timeDependenceDecimalOffset, stream);

        WriteJudgments(config.judgments, stream);
        WriteJudgments(config.chainHeadJudgments, stream);
        if (config.chainLinkDisplay.has_value()) WriteColoredText(config.chainLinkDisplay.value(), stream);
        WriteSegments(config.beforeCutAngleJudgments, stream);
        WriteSegments(config.accuracyJudgments, stream);
        WriteSegments(config.afterCutAngleJudgments, stream);
        WriteTimeSegments(config.timeDependenceJudgments, stream);
        WriteBadCutDisplays(config.badCutDisplays, stream);
        WriteColoredTexts(config.missDisplays, stream);
        return stream;
    }
} // namespace

namespace SnoreSaber::Data::Private::HsvReplayConfigCodec
{
    bool TryEncodeJson(std::string_view json, std::vector<char>& payload, std::string& failure)
    {
        payload.clear();
        failure.clear();

        ConfigJson config;
        try
        {
            config = ParseConfig(json);
        }
        catch (const std::exception& exception)
        {
            failure = std::string("failed to parse HSV config: ") + exception.what();
            return false;
        }

        HsvReplayConfig replayConfig;
        if (!TryNormalize(config, replayConfig, failure))
        {
            return false;
        }

        payload = Encode(replayConfig);
        if ((int)payload.size() > MaxPayloadBytes)
        {
            failure = "HSV config payload is too large";
            payload.clear();
            return false;
        }

        return true;
    }
} // namespace SnoreSaber::Data::Private::HsvReplayConfigCodec
