#include "Features/Replays/Format/HsvReplayConfigCodec.hpp"
#include "Features/Replays/Format/ReplayReader.hpp"
#include "Features/Replays/Format/ReplayWriter.hpp"
#include "Features/Replays/ReplayLimits.hpp"
#include "Utils/lzma/lzma.hpp"

#include <cstring>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace ScoreSaber::Data::Private;

namespace
{
    [[noreturn]] void Fail(std::string_view message)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }

    void Require(bool condition, std::string_view message)
    {
        if (!condition)
        {
            Fail(message);
        }
    }

    const std::string& ReplayMagic()
    {
        static const std::string magic = std::string("ScoreSaber Replay ") + "\xf0\x9f\x91\x8c\xf0\x9f\xa4\xa0\r\n";
        return magic;
    }

    VRPosition PositionFor(int seed)
    {
        return VRPosition(seed * 0.1f, seed * -0.2f, seed * 0.3f);
    }

    VRRotation RotationFor(int seed)
    {
        return VRRotation(seed * 0.01f, seed * -0.02f, seed * 0.03f, 1.0f - seed * 0.001f);
    }

    VRPose PoseFor(int seed)
    {
        return VRPose(PositionFor(seed), RotationFor(seed));
    }

    VRPoseGroup PoseGroupFor(int seed)
    {
        return VRPoseGroup(PoseFor(seed), PoseFor(seed + 1), PoseFor(seed + 2), 72 + seed % 3, seed * 0.02f);
    }

    NoteEvent NoteEventFor(int seed)
    {
        return NoteEvent(
            NoteID(seed * 0.25f, seed % 3, seed % 4, seed % 2, seed % 8, seed % 4, seed % 9 - 1, seed * 1.5f),
            static_cast<NoteEventType>(1 + seed % 4),
            PositionFor(seed + 10),
            PositionFor(seed + 20),
            PositionFor(seed + 30),
            seed % 2,
            seed % 2 == 0,
            12.0f + seed,
            45.0f + seed,
            0.5f + seed * 0.01f,
            1.5f + seed * 0.01f,
            0.7f + seed * 0.01f,
            0.8f + seed * 0.01f,
            seed * 0.25f,
            1.0f,
            1.0f,
            seed * -0.01f,
            RotationFor(seed + 40),
            RotationFor(seed + 50),
            RotationFor(seed + 60),
            PositionFor(seed + 70));
    }

    std::shared_ptr<ReplayFile> MakeReplayFile()
    {
        std::vector<VRPoseGroup> poses;
        poses.reserve(240);
        for (int i = 0; i < 240; ++i)
        {
            poses.push_back(PoseGroupFor(i));
        }

        std::vector<HeightEvent> heights = {
            HeightEvent(1.7f, 0.0f),
            HeightEvent(1.72f, 15.5f),
            HeightEvent(1.69f, 31.0f),
        };

        std::vector<NoteEvent> notes = {
            NoteEventFor(1),
            NoteEventFor(2),
            NoteEventFor(3),
        };

        std::vector<ScoreEvent> scores = {
            ScoreEvent(115, 0.5f, 115),
            ScoreEvent(230, 1.0f, 230),
            ScoreEvent(345, 1.5f, 345),
            ScoreEvent(460, 2.0f, 460),
        };

        std::vector<ComboEvent> combos = {
            ComboEvent(1, 0.5f),
            ComboEvent(2, 1.0f),
            ComboEvent(3, 1.5f),
        };

        std::vector<MultiplierEvent> multipliers = {
            MultiplierEvent(1, 0.0f, 0.0f),
            MultiplierEvent(2, 0.5f, 3.0f),
            MultiplierEvent(4, 0.25f, 6.0f),
        };

        std::vector<EnergyEvent> energies = {
            EnergyEvent(0.5f, 0.0f),
            EnergyEvent(0.65f, 3.0f),
            EnergyEvent(0.8f, 6.0f),
        };

        auto metadata = std::make_shared<Metadata>(
            version("3.1.0"),
            "custom_level_0123456789abcdef",
            7,
            "Standard",
            "DefaultEnvironment",
            std::vector<std::string>{"DA", "FS", "GN"},
            0.35f,
            false,
            1.7f,
            12.5f,
            VRPosition(1.0f, 2.0f, 3.0f),
            -1.0f,
            version("1.40.8"),
            version("1.0.0"),
            std::string("Quest"));

        return std::make_shared<ReplayFile>(
            metadata,
            poses,
            heights,
            notes,
            scores,
            combos,
            multipliers,
            energies);
    }

    std::shared_ptr<ReplayFile> MakeReplayFileWithExtensions()
    {
        auto file = MakeReplayFile();
        auto& metadata = *file->metadata;

        metadata.HasPlaySettingsExtension = true;
        metadata.SongSpeed = 1.2f;
        metadata.JumpDistance = 22.75f;
        metadata.LeftSaberColor = UnityEngine::Color(0.9f, 0.1f, 0.1f, 1.0f);
        metadata.RightSaberColor = UnityEngine::Color(0.1f, 0.1f, 0.9f, 1.0f);
        // ObstacleColor stays nullopt to cover the hasValue=false path
        metadata.EnvironmentColor0 = UnityEngine::Color(0.8f, 0.2f, 0.3f, 1.0f);
        metadata.EnvironmentColor1 = UnityEngine::Color(0.3f, 0.2f, 0.8f, 1.0f);
        metadata.EnvironmentColorW = UnityEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        metadata.EnvironmentColor0Boost = UnityEngine::Color(0.7f, 0.4f, 0.2f, 1.0f);
        metadata.EnvironmentColor1Boost = UnityEngine::Color(0.2f, 0.4f, 0.7f, 1.0f);
        metadata.EnvironmentColorWBoost = UnityEngine::Color(0.5f, 0.5f, 0.5f, 1.0f);
        metadata.SupportsEnvironmentColorBoost = true;
        metadata.EnvironmentEffectsFilterDefaultPreset = 1;
        metadata.EnvironmentEffectsFilterExpertPlusPreset = 2;
        metadata.EnvironmentEffectsFilterPreset = 2;
        metadata.NoTextsAndHuds = true;
        metadata.SaberTrailIntensity = 0.65f;
        metadata.HideNoteSpawnEffect = true;
        metadata.ArcsHapticFeedback = true;
        metadata.ArcVisibility = 2;

        ReplayControllerOffsets offsets;
        // Shared stays nullopt to cover the hasValue=false path
        offsets.Left = ReplayControllerOffset{VRPosition(0.01f, -0.02f, 0.03f), VRPosition(1.5f, -2.5f, 3.5f)};
        offsets.Right = ReplayControllerOffset{VRPosition(-0.01f, 0.02f, -0.03f), VRPosition(-1.5f, 2.5f, -3.5f)};
        metadata.ControllerOffsets = offsets;

        file->pauseKeyframes = {
            PauseEvent{12.5f, 4200, 1750000000000, 1750000004200},
            PauseEvent{31.0f, 800, 1750000050000, 1750000050800},
        };
        file->wallKeyframes = {
            WallEvent{5.0f, 5.4f, 0.85f, 4.75f, 1.0f, 1, 0, 2, 3},
        };

        const std::string hsv = R"({"judgments":[{"threshold":115,"text":"Fantastic"}]})";
        file->hsvConfig = std::vector<char>(hsv.begin(), hsv.end());

        return file;
    }

    void ExpectEqual(int expected, int actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(bool expected, bool actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(float expected, float actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::string& expected, const std::string& actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const version& expected, const version& actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::optional<int>& expected, const std::optional<int>& actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::optional<float>& expected, const std::optional<float>& actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::optional<std::string>& expected, const std::optional<std::string>& actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::vector<std::string>& expected, const std::vector<std::string>& actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::optional<version>& expected, const std::optional<version>& actual, std::string_view label)
    {
        Require(expected.has_value() == actual.has_value(), label);
        if (expected)
        {
            ExpectEqual(*expected, *actual, label);
        }
    }

    void ExpectEqual(const VRPosition& expected, const VRPosition& actual, std::string_view label)
    {
        ExpectEqual(expected.X, actual.X, label);
        ExpectEqual(expected.Y, actual.Y, label);
        ExpectEqual(expected.Z, actual.Z, label);
    }

    void ExpectEqual(const VRRotation& expected, const VRRotation& actual, std::string_view label)
    {
        ExpectEqual(expected.X, actual.X, label);
        ExpectEqual(expected.Y, actual.Y, label);
        ExpectEqual(expected.Z, actual.Z, label);
        ExpectEqual(expected.W, actual.W, label);
    }

    void ExpectEqual(const std::optional<VRPosition>& expected, const std::optional<VRPosition>& actual, std::string_view label)
    {
        Require(expected.has_value() == actual.has_value(), label);
        if (expected)
        {
            ExpectEqual(*expected, *actual, label);
        }
    }

    void ExpectEqual(const std::optional<VRRotation>& expected, const std::optional<VRRotation>& actual, std::string_view label)
    {
        Require(expected.has_value() == actual.has_value(), label);
        if (expected)
        {
            ExpectEqual(*expected, *actual, label);
        }
    }

    void ExpectEqual(const VRPose& expected, const VRPose& actual, std::string_view label)
    {
        ExpectEqual(expected.Position, actual.Position, label);
        ExpectEqual(expected.Rotation, actual.Rotation, label);
    }

    void ExpectEqual(const VRPoseGroup& expected, const VRPoseGroup& actual, std::string_view label)
    {
        ExpectEqual(expected.Head, actual.Head, label);
        ExpectEqual(expected.Left, actual.Left, label);
        ExpectEqual(expected.Right, actual.Right, label);
        ExpectEqual(expected.FPS, actual.FPS, label);
        ExpectEqual(expected.Time, actual.Time, label);
    }

    void ExpectEqual(const HeightEvent& expected, const HeightEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Height, actual.Height, label);
        ExpectEqual(expected.Time, actual.Time, label);
    }

    void ExpectEqual(const NoteID& expected, const NoteID& actual, std::string_view label)
    {
        ExpectEqual(expected.Time, actual.Time, label);
        ExpectEqual(expected.LineLayer, actual.LineLayer, label);
        ExpectEqual(expected.LineIndex, actual.LineIndex, label);
        ExpectEqual(expected.ColorType, actual.ColorType, label);
        ExpectEqual(expected.CutDirection, actual.CutDirection, label);
        ExpectEqual(expected.GameplayType, actual.GameplayType, label);
        ExpectEqual(expected.ScoringType, actual.ScoringType, label);
        ExpectEqual(expected.CutDirectionAngleOffset, actual.CutDirectionAngleOffset, label);
    }

    void ExpectEqual(const NoteEvent& expected, const NoteEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.TheNoteID, actual.TheNoteID, label);
        ExpectEqual(static_cast<int>(expected.EventType), static_cast<int>(actual.EventType), label);
        ExpectEqual(expected.CutPoint, actual.CutPoint, label);
        ExpectEqual(expected.CutNormal, actual.CutNormal, label);
        ExpectEqual(expected.SaberDirection, actual.SaberDirection, label);
        ExpectEqual(expected.SaberType, actual.SaberType, label);
        ExpectEqual(expected.DirectionOK, actual.DirectionOK, label);
        ExpectEqual(expected.SaberSpeed, actual.SaberSpeed, label);
        ExpectEqual(expected.CutAngle, actual.CutAngle, label);
        ExpectEqual(expected.CutDistanceToCenter, actual.CutDistanceToCenter, label);
        ExpectEqual(expected.CutDirectionDeviation, actual.CutDirectionDeviation, label);
        ExpectEqual(expected.BeforeCutRating, actual.BeforeCutRating, label);
        ExpectEqual(expected.AfterCutRating, actual.AfterCutRating, label);
        ExpectEqual(expected.Time, actual.Time, label);
        ExpectEqual(expected.UnityTimescale, actual.UnityTimescale, label);
        ExpectEqual(expected.TimeSyncTimescale, actual.TimeSyncTimescale, label);
        ExpectEqual(expected.TimeDeviation, actual.TimeDeviation, label);
        ExpectEqual(expected.WorldRotation, actual.WorldRotation, label);
        ExpectEqual(expected.InverseWorldRotation, actual.InverseWorldRotation, label);
        ExpectEqual(expected.NoteRotation, actual.NoteRotation, label);
        ExpectEqual(expected.NotePosition, actual.NotePosition, label);
    }

    void ExpectEqual(const ScoreEvent& expected, const ScoreEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Score, actual.Score, label);
        ExpectEqual(expected.Time, actual.Time, label);
        ExpectEqual(expected.ImmediateMaxPossibleScore, actual.ImmediateMaxPossibleScore, label);
    }

    void ExpectEqual(const ComboEvent& expected, const ComboEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Combo, actual.Combo, label);
        ExpectEqual(expected.Time, actual.Time, label);
    }

    void ExpectEqual(const MultiplierEvent& expected, const MultiplierEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Multiplier, actual.Multiplier, label);
        ExpectEqual(expected.NextMultiplierProgress, actual.NextMultiplierProgress, label);
        ExpectEqual(expected.Time, actual.Time, label);
    }

    void ExpectEqual(const EnergyEvent& expected, const EnergyEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Energy, actual.Energy, label);
        ExpectEqual(expected.Time, actual.Time, label);
    }

    void ExpectEqual(std::int64_t expected, std::int64_t actual, std::string_view label)
    {
        Require(expected == actual, label);
    }

    void ExpectEqual(const std::optional<UnityEngine::Color>& expected, const std::optional<UnityEngine::Color>& actual, std::string_view label)
    {
        Require(expected.has_value() == actual.has_value(), label);
        if (expected)
        {
            ExpectEqual(expected->r, actual->r, label);
            ExpectEqual(expected->g, actual->g, label);
            ExpectEqual(expected->b, actual->b, label);
            ExpectEqual(expected->a, actual->a, label);
        }
    }

    void ExpectEqual(const PauseEvent& expected, const PauseEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Time, actual.Time, label);
        ExpectEqual(expected.Duration, actual.Duration, label);
        ExpectEqual(expected.UnixStartTime, actual.UnixStartTime, label);
        ExpectEqual(expected.UnixEndTime, actual.UnixEndTime, label);
    }

    void ExpectEqual(const WallEvent& expected, const WallEvent& actual, std::string_view label)
    {
        ExpectEqual(expected.Time, actual.Time, label);
        ExpectEqual(expected.ExitTime, actual.ExitTime, label);
        ExpectEqual(expected.Energy, actual.Energy, label);
        ExpectEqual(expected.ObstacleTime, actual.ObstacleTime, label);
        ExpectEqual(expected.ObstacleDuration, actual.ObstacleDuration, label);
        ExpectEqual(expected.LineIndex, actual.LineIndex, label);
        ExpectEqual(expected.LineLayer, actual.LineLayer, label);
        ExpectEqual(expected.Width, actual.Width, label);
        ExpectEqual(expected.Height, actual.Height, label);
    }

    void ExpectEqual(const std::optional<ReplayControllerOffset>& expected, const std::optional<ReplayControllerOffset>& actual, std::string_view label)
    {
        Require(expected.has_value() == actual.has_value(), label);
        if (expected)
        {
            ExpectEqual(expected->Position, actual->Position, label);
            ExpectEqual(expected->Rotation, actual->Rotation, label);
        }
    }

    void ExpectEqual(const std::optional<ReplayControllerOffsets>& expected, const std::optional<ReplayControllerOffsets>& actual, std::string_view label)
    {
        Require(expected.has_value() == actual.has_value(), label);
        if (expected)
        {
            ExpectEqual(expected->Shared, actual->Shared, label);
            ExpectEqual(expected->Left, actual->Left, label);
            ExpectEqual(expected->Right, actual->Right, label);
        }
    }

    template <typename T>
    void ExpectVectorEqual(const std::vector<T>& expected, const std::vector<T>& actual, std::string_view label)
    {
        Require(expected.size() == actual.size(), label);
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            ExpectEqual(expected[i], actual[i], label);
        }
    }

    void ExpectEqual(const Metadata& expected, const Metadata& actual)
    {
        ExpectEqual(expected.Version, actual.Version, "metadata version");
        ExpectEqual(expected.LevelID, actual.LevelID, "metadata level id");
        ExpectEqual(expected.Difficulty, actual.Difficulty, "metadata difficulty");
        ExpectEqual(expected.Characteristic, actual.Characteristic, "metadata characteristic");
        ExpectEqual(expected.Environment, actual.Environment, "metadata environment");
        ExpectEqual(expected.Modifiers, actual.Modifiers, "metadata modifiers");
        ExpectEqual(expected.NoteSpawnOffset, actual.NoteSpawnOffset, "metadata note spawn offset");
        ExpectEqual(expected.LeftHanded, actual.LeftHanded, "metadata handedness");
        ExpectEqual(expected.InitialHeight, actual.InitialHeight, "metadata initial height");
        ExpectEqual(expected.RoomRotation, actual.RoomRotation, "metadata room rotation");
        ExpectEqual(expected.RoomCenter, actual.RoomCenter, "metadata room center");
        ExpectEqual(expected.FailTime, actual.FailTime, "metadata fail time");
        ExpectEqual(expected.GameVersion, actual.GameVersion, "metadata game version");
        ExpectEqual(expected.PluginVersion, actual.PluginVersion, "metadata plugin version");
        ExpectEqual(expected.Platform, actual.Platform, "metadata platform");
        ExpectEqual(expected.HasPlaySettingsExtension, actual.HasPlaySettingsExtension, "metadata play settings flag");
        ExpectEqual(expected.SongSpeed, actual.SongSpeed, "metadata song speed");
        ExpectEqual(expected.JumpDistance, actual.JumpDistance, "metadata jump distance");
        ExpectEqual(expected.LeftSaberColor, actual.LeftSaberColor, "metadata left saber color");
        ExpectEqual(expected.RightSaberColor, actual.RightSaberColor, "metadata right saber color");
        ExpectEqual(expected.ObstacleColor, actual.ObstacleColor, "metadata obstacle color");
        ExpectEqual(expected.EnvironmentColor0, actual.EnvironmentColor0, "metadata environment color 0");
        ExpectEqual(expected.EnvironmentColor1, actual.EnvironmentColor1, "metadata environment color 1");
        ExpectEqual(expected.EnvironmentColorW, actual.EnvironmentColorW, "metadata environment color w");
        ExpectEqual(expected.EnvironmentColor0Boost, actual.EnvironmentColor0Boost, "metadata environment color 0 boost");
        ExpectEqual(expected.EnvironmentColor1Boost, actual.EnvironmentColor1Boost, "metadata environment color 1 boost");
        ExpectEqual(expected.EnvironmentColorWBoost, actual.EnvironmentColorWBoost, "metadata environment color w boost");
        ExpectEqual(expected.SupportsEnvironmentColorBoost, actual.SupportsEnvironmentColorBoost, "metadata color boost support");
        ExpectEqual(expected.EnvironmentEffectsFilterDefaultPreset, actual.EnvironmentEffectsFilterDefaultPreset, "metadata effects default preset");
        ExpectEqual(expected.EnvironmentEffectsFilterExpertPlusPreset, actual.EnvironmentEffectsFilterExpertPlusPreset, "metadata effects expert plus preset");
        ExpectEqual(expected.EnvironmentEffectsFilterPreset, actual.EnvironmentEffectsFilterPreset, "metadata effects preset");
        ExpectEqual(expected.NoTextsAndHuds, actual.NoTextsAndHuds, "metadata no texts and huds");
        ExpectEqual(expected.SaberTrailIntensity, actual.SaberTrailIntensity, "metadata saber trail intensity");
        ExpectEqual(expected.HideNoteSpawnEffect, actual.HideNoteSpawnEffect, "metadata hide note spawn effect");
        ExpectEqual(expected.ArcsHapticFeedback, actual.ArcsHapticFeedback, "metadata arcs haptic feedback");
        ExpectEqual(expected.ArcVisibility, actual.ArcVisibility, "metadata arc visibility");
        ExpectEqual(expected.ControllerOffsets, actual.ControllerOffsets, "metadata controller offsets");
    }

    void ExpectEqual(const ReplayFile& expected, const ReplayFile& actual)
    {
        Require(expected.metadata != nullptr, "expected metadata");
        Require(actual.metadata != nullptr, "actual metadata");
        ExpectEqual(*expected.metadata, *actual.metadata);
        ExpectVectorEqual(expected.poseKeyframes, actual.poseKeyframes, "pose keyframes");
        ExpectVectorEqual(expected.heightKeyframes, actual.heightKeyframes, "height keyframes");
        ExpectVectorEqual(expected.noteKeyframes, actual.noteKeyframes, "note keyframes");
        ExpectVectorEqual(expected.scoreKeyframes, actual.scoreKeyframes, "score keyframes");
        ExpectVectorEqual(expected.comboKeyframes, actual.comboKeyframes, "combo keyframes");
        ExpectVectorEqual(expected.multiplierKeyframes, actual.multiplierKeyframes, "multiplier keyframes");
        ExpectVectorEqual(expected.energyKeyframes, actual.energyKeyframes, "energy keyframes");
        ExpectVectorEqual(expected.pauseKeyframes, actual.pauseKeyframes, "pause keyframes");
        ExpectVectorEqual(expected.wallKeyframes, actual.wallKeyframes, "wall keyframes");
        Require(expected.hsvConfig == actual.hsvConfig, "hsv config");
    }

    std::vector<char> DecompressReplay(const std::vector<char>& replay)
    {
        const auto& magic = ReplayMagic();
        Require(replay.size() > magic.size(), "replay has compressed payload");
        Require(std::string(replay.begin(), replay.begin() + magic.size()) == magic, "replay magic");

        std::vector<char> compressed(replay.begin() + magic.size(), replay.end());
        std::vector<char> decompressed;
        Require(LZMA::lzmaDecompress(compressed, decompressed, ScoreSaber::ReplaySystem::ReplayLimits::MaxDecompressedReplayBytes), "decompress replay");
        return decompressed;
    }

    std::vector<char> CompressReplay(const std::vector<char>& decompressed)
    {
        const auto& magic = ReplayMagic();
        std::vector<char> compressed;
        Require(LZMA::lzmaCompress(decompressed, compressed), "compress replay");

        std::vector<char> replay(magic.begin(), magic.end());
        replay.insert(replay.end(), compressed.begin(), compressed.end());
        return replay;
    }

    void ExpectRejected(const std::vector<char>& replay, std::string_view label)
    {
        bool threw = false;
        std::shared_ptr<ReplayFile> parsed;
        try
        {
            parsed = ReplayReader::Read(replay);
        }
        catch (...)
        {
            threw = true;
        }

        Require(threw || parsed == nullptr, label);
    }

    void TestScoringTypeStub()
    {
        const NoteID sliderHead(0.0f, 0, 0, 0, 0, 0, static_cast<int>(ScoringType_pre1_40::SliderHead), 0.0f);
        Require(sliderHead.MatchesScoringType(GlobalNamespace::NoteData_ScoringType::ArcHeadArcTail, version("1.39.0")), "pre-1.40 slider head scoring compatibility");
        Require(!sliderHead.MatchesScoringType(GlobalNamespace::NoteData_ScoringType::ChainHead, version("1.39.0")), "pre-1.40 slider head scoring mismatch");

        const NoteID chainLink(0.0f, 0, 0, 0, 0, 0, static_cast<int>(GlobalNamespace::NoteData_ScoringType::ChainLink), 0.0f);
        Require(chainLink.MatchesScoringType(GlobalNamespace::NoteData_ScoringType::ChainLink, version("1.40.0")), "1.40 scoring compatibility");
    }

    std::vector<char> RequireHsvEncodes(std::string_view json, std::string_view what)
    {
        std::vector<char> payload;
        std::string failure;
        if (!HsvReplayConfigCodec::TryEncodeJson(json, payload, failure))
        {
            Fail(std::string(what) + " (failure: " + failure + ")");
        }
        return payload;
    }

    void RequireHsvRejects(std::string_view json, std::string_view expectedFailure, std::string_view what)
    {
        std::vector<char> payload;
        std::string failure;
        Require(!HsvReplayConfigCodec::TryEncodeJson(json, payload, failure), std::string(what) + " (unexpectedly encoded)");
        Require(payload.empty(), std::string(what) + " (payload not cleared)");
        if (failure != expectedFailure)
        {
            Fail(std::string(what) + " (failure was: " + failure + ")");
        }
    }

    void TestHsvConfigCodec()
    {
        // byte-exact payload for a config exercising case-insensitive keys, string enums,
        // array + object colors, out-of-order judgments and the default chain head
        const std::string json = R"({
            "MAJORVERSION": 3,
            "minorVersion": 2,
            "patchVersion": 1,
            "displayMode": "format",
            "fixedPosition": {"x": 1.0, "y": 2.0, "z": 3.0},
            "assumeMaxPostSwing": false,
            "judgments": [
                {"threshold": 100, "text": "B", "color": [0.0, 1.0, 0.0, 1.0], "fade": true},
                {"threshold": 110, "text": "A", "color": {"r": 1.0, "g": 0.0, "b": 0.5, "a": 1.0}}
            ],
            "accuracyJudgments": [{"threshold": 15, "text": "+"}],
            "timeDependenceJudgments": [{"threshold": 0.5, "text": "t"}],
            "badCutDisplays": [{"text": "bad", "type": "Bomb"}],
            "randomizeBadCutDisplays": false,
            "missDisplays": [{"text": "miss"}]
        })";
        auto payload = RequireHsvEncodes(json, "hsv config encoded");

        std::vector<char> expected;
        auto putByte = [&expected](uint8_t value) { expected.push_back(static_cast<char>(value)); };
        auto putUShort = [&](int value) { putByte(value & 0xff); putByte((value >> 8) & 0xff); };
        auto putFloat = [&expected](float value) {
            char bytes[sizeof(float)];
            std::memcpy(bytes, &value, sizeof(float));
            expected.insert(expected.end(), bytes, bytes + sizeof(float));
        };
        auto putString = [&](std::string_view value) {
            putUShort(static_cast<int>(value.size()));
            expected.insert(expected.end(), value.begin(), value.end());
        };

        putByte(3); putByte(2); putByte(1);              // version 3.2.1
        putByte(1);                                      // displayMode Format
        putByte(1 | 4 | 64);                             // fixedPosition + doIntermediateUpdates + randomizeMissDisplays
        putFloat(1.0f); putFloat(2.0f); putFloat(3.0f);  // fixedPosition
        putByte(1); putByte(2);                          // td precision/offset defaults
        putByte(2);                                      // judgments, sorted descending
        putUShort(110); putString("A"); putByte(255); putByte(0); putByte(128); putByte(255); putByte(0);
        putUShort(100); putString("B"); putByte(0); putByte(255); putByte(0); putByte(255); putByte(1);
        putByte(1);                                      // default chain head judgment
        putUShort(0); putString("%s"); putByte(255); putByte(255); putByte(255); putByte(255); putByte(0);
        putByte(0);                                      // beforeCutAngleJudgments
        putByte(1); putUShort(15); putString("+");       // accuracyJudgments
        putByte(0);                                      // afterCutAngleJudgments
        putByte(1); putFloat(0.5f); putString("t");      // timeDependenceJudgments
        putByte(1); putString("bad"); putByte(255); putByte(255); putByte(255); putByte(255); putByte(3); // badCutDisplays (Bomb)
        putByte(1); putString("miss"); putByte(255); putByte(255); putByte(255); putByte(255);            // missDisplays

        Require(payload == expected, "hsv payload bytes match the wire format");

        // minimal config falls back to every default
        auto minimal = RequireHsvEncodes(R"({"majorVersion": 3, "judgments": [{"threshold": 0, "text": "x"}]})", "minimal hsv config encoded");
        Require(static_cast<uint8_t>(minimal[3]) == 3, "default display mode is Numeric");
        Require(static_cast<uint8_t>(minimal[4]) == (4 | 32 | 64), "default flags set randomize + intermediate updates");

        const std::string judgment = R"([{"threshold": 0, "text": "x"}])";
        RequireHsvRejects(R"({"majorVersion": 2, "judgments": )" + judgment + "}", "unsupported HSV config version", "wrong major version rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "minorVersion": 8, "judgments": )" + judgment + "}", "unsupported HSV config version", "future minor version rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "displayMode": 9, "judgments": )" + judgment + "}", "unsupported HSV display mode", "unknown display mode rejected");
        RequireHsvRejects(R"({"majorVersion": 3})", "HSV judgments are empty", "missing judgments rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "judgments": [{"threshold": 5, "text": "x", "fade": true}]})", "first HSV judgments entry cannot fade", "fading first judgment rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "judgments": [{"threshold": 5, "text": "x"}, {"threshold": 5, "text": "y"}]})", "HSV judgments contain duplicate thresholds", "duplicate thresholds rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "judgments": [{"threshold": 70000, "text": "x"}]})", "HSV judgments contain an out of range threshold", "oversized threshold rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "judgments": [{"threshold": 0, "text": ")" + std::string(513, 'a') + R"("}]})", "HSV judgments contain text that is too long", "oversized judgment text rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "judgments": [null]})", "HSV judgments contain an empty entry", "null judgment entry rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "timeDependenceDecimalPrecision": 100, "judgments": )" + judgment + "}", "HSV time dependence decimal precision is out of range", "precision out of range rejected");

        std::string tooMany = R"({"majorVersion": 3, "judgments": [{"threshold": 33, "text": "x"})";
        for (int i = 32; i > 0; --i)
        {
            tooMany += R"(, {"threshold": )" + std::to_string(i) + R"(, "text": "x"})";
        }
        tooMany += "]}";
        RequireHsvRejects(tooMany, "HSV judgments contain too many entries", "too many judgments rejected");

        RequireHsvRejects(R"(not json)", "failed to parse HSV config: invalid JSON document", "malformed json rejected");
        RequireHsvRejects(R"({"majorVersion": 3, "judgments": [{"threshold": 0, "text": "x", "color": true}]})", "failed to parse HSV config: Invalid HSV color", "bad color shape rejected");

        // payload cap: 32 judgments with max-length texts blow past 8KB while every field is individually valid
        std::string huge = R"({"majorVersion": 3, "judgments": [)";
        for (int i = 0; i < 32; ++i)
        {
            if (i > 0)
            {
                huge += ",";
            }
            huge += R"({"threshold": )" + std::to_string(320 - i * 10) + R"(, "text": ")" + std::string(512, 'a') + R"("})";
        }
        huge += "]}";
        RequireHsvRejects(huge, "HSV config payload is too large", "oversized payload rejected");
    }
} // namespace

int main()
{
    TestScoringTypeStub();
    TestHsvConfigCodec();

    auto expected = MakeReplayFile();
    auto encoded = ReplayWriter::Write(expected);
    Require(!encoded.empty(), "writer produced bytes");

    auto actual = ReplayReader::Read(encoded);
    Require(actual != nullptr, "reader parsed writer output");
    // the writer emits a play-settings extension whenever Environment is set, so the flag flips on
    expected->metadata->HasPlaySettingsExtension = true;
    ExpectEqual(*expected, *actual);

    auto expectedExtended = MakeReplayFileWithExtensions();
    auto encodedExtended = ReplayWriter::Write(expectedExtended);
    auto actualExtended = ReplayReader::Read(encodedExtended);
    Require(actualExtended != nullptr, "reader parsed extended writer output");
    ExpectEqual(*expectedExtended, *actualExtended);

    // legacy replays carry a zeroed extensions pointer; they must parse with extension defaults
    auto legacy = DecompressReplay(encodedExtended);
    const int zeroPointer = 0;
    std::memcpy(legacy.data() + sizeof(int) * 8, &zeroPointer, sizeof(zeroPointer));
    auto legacyParsed = ReplayReader::Read(CompressReplay(legacy));
    Require(legacyParsed != nullptr, "legacy replay parsed");
    Require(!legacyParsed->metadata->HasPlaySettingsExtension, "legacy replay has no play settings");
    Require(legacyParsed->pauseKeyframes.empty(), "legacy replay has no pause keyframes");
    Require(legacyParsed->hsvConfig.empty(), "legacy replay has no hsv config");

    // a bogus extensions pointer must not reject the replay, only drop the extensions
    auto corruptedExtensions = DecompressReplay(encodedExtended);
    const int bogusPointer = static_cast<int>(corruptedExtensions.size()) - 3;
    std::memcpy(corruptedExtensions.data() + sizeof(int) * 8, &bogusPointer, sizeof(bogusPointer));
    auto tolerated = ReplayReader::Read(CompressReplay(corruptedExtensions));
    Require(tolerated != nullptr, "corrupt extensions tolerated");
    Require(!tolerated->metadata->HasPlaySettingsExtension, "corrupt extensions dropped");

    auto truncated = encoded;
    truncated.resize(truncated.size() - 4);
    ExpectRejected(truncated, "truncated replay rejected");

    auto decompressed = DecompressReplay(encoded);
    const int invalidMetadataOffset = static_cast<int>(decompressed.size() + 1024);
    std::memcpy(decompressed.data(), &invalidMetadataOffset, sizeof(invalidMetadataOffset));
    ExpectRejected(CompressReplay(decompressed), "corrupted pointer table rejected");

    std::cout << "PASS\n";
    return 0;
}
