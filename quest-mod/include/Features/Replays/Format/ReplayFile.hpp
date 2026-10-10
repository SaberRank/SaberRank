#pragma once

#include <memory>
#include <optional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <GlobalNamespace/NoteData.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/Quaternion.hpp>
#include <UnityEngine/Vector3.hpp>

#include "Features/Replays/Format/ReplayScoringTypes.hpp"
#include "Utils/Versions.hpp"

using namespace std;

namespace SnoreSaber::Data::Private
{
    struct Pointers
    {
        Pointers();
        Pointers(int metadata, int poseKeyframes, int heightKeyframes, int noteKeyframes, int scoreKeyframes, int comboKeyframes, int multiplierKeyframes, int energyKeyframes, int extensions);
        int metadata;
        int poseKeyframes;
        int heightKeyframes;
        int noteKeyframes;
        int scoreKeyframes;
        int comboKeyframes;
        int multiplierKeyframes;
        int energyKeyframes;
        int extensions;
    };

    struct VRPosition
    {
        VRPosition();
        VRPosition(float X, float Y, float Z);
        VRPosition(UnityEngine::Vector3 vector);
        float X;
        float Y;
        float Z;
        void Mirror();
    };

    struct VRRotation
    {
        VRRotation();
        VRRotation(float X, float Y, float Z, float W);
        VRRotation(UnityEngine::Quaternion quaternion);
        float X;
        float Y;
        float Z;
        float W;
        void Mirror();
    };

    struct VRPose
    {
        VRPose();
        VRPose(VRPosition Position, VRRotation Rotation);
        VRPosition Position;
        VRRotation Rotation;
        void Mirror();
    };

    struct ScoreEvent
    {
        ScoreEvent();
        ScoreEvent(int Score, float Time);
        ScoreEvent(int Score, float Time, int ImmediateMaxPossible);
        int Score;
        float Time;
        std::optional<int> ImmediateMaxPossibleScore;
    };

    struct ComboEvent
    {
        ComboEvent();
        ComboEvent(int Combo, float Time);
        int Combo;
        float Time;
    };

    enum NoteEventType
    {
        None,
        GoodCut,
        BadCut,
        Miss,
        Bomb
    };

    struct NoteID
    {
        NoteID();
        NoteID(float Time, int LineLayer, int LineIndex, int ColorType, int CutDirection);
        NoteID(float Time, int LineLayer, int LineIndex, int ColorType, int CutDirection, int GameplayType, int ScoringType, float CutDirectionAngleOffset);
        float Time;
        int LineLayer;
        int LineIndex;
        int ColorType;
        int CutDirection;
        std::optional<int> GameplayType;
        std::optional<int> ScoringType;
        std::optional<float> CutDirectionAngleOffset;

        bool MatchesScoringType(GlobalNamespace::NoteData_ScoringType comparedScoringType, optional<version> gameVersion) const;
        void Mirror();
    };

    struct EnergyEvent
    {
        EnergyEvent();
        EnergyEvent(float Energy, float Time);
        float Energy;
        float Time;
    };

    struct HeightEvent
    {
        HeightEvent();
        HeightEvent(float Height, float Time);
        float Height;
        float Time;
    };

    struct MultiplierEvent
    {
        MultiplierEvent();
        MultiplierEvent(int Multiplier, float NextMultiplierProgress, float Time);
        int Multiplier;
        float NextMultiplierProgress;
        float Time;
    };

    struct VRPoseGroup
    {
        VRPoseGroup();
        VRPoseGroup(VRPose Head, VRPose Left, VRPose Right, int FPS, float Time);
        VRPose Head;
        VRPose Left;
        VRPose Right;
        int FPS;
        float Time;
        void Mirror();
    };

    struct ReplayControllerOffset
    {
        VRPosition Position;
        VRPosition Rotation;
    };

    struct ReplayControllerOffsets
    {
        std::optional<ReplayControllerOffset> Shared;
        std::optional<ReplayControllerOffset> Left;
        std::optional<ReplayControllerOffset> Right;
    };

    struct PauseEvent
    {
        float Time;
        int64_t Duration;
        int64_t UnixStartTime;
        int64_t UnixEndTime;
    };

    struct WallEvent
    {
        float Time;
        float ExitTime;
        float Energy;
        float ObstacleTime;
        float ObstacleDuration;
        int LineIndex;
        int LineLayer;
        int Width;
        int Height;
    };

    struct Metadata
    {
        Metadata();
        Metadata(version Version, string LevelID, int Difficulty, string Characteristic, string Environment, vector<string> Modifiers, float NoteSpawnOffset,
                 bool LeftHanded, float InitialHeight, float RoomRotation, VRPosition RoomCenter, float FailTime, optional<version> GameVersion, optional<version> PluginVersion, optional<string> Platform);
        version Version;
        string LevelID;
        int Difficulty;
        string Characteristic;
        string Environment;
        vector<string> Modifiers;
        float NoteSpawnOffset;
        bool LeftHanded;
        float InitialHeight;
        float RoomRotation;
        VRPosition RoomCenter;
        float FailTime;
        optional<version> GameVersion;
        optional<version> PluginVersion;
        optional<string> Platform;

        // official replay metadata ends at Platform; fields below map from extension payloads
        bool HasPlaySettingsExtension = false;
        float SongSpeed = 0.0f;
        float JumpDistance = 0.0f;
        optional<UnityEngine::Color> LeftSaberColor;
        optional<UnityEngine::Color> RightSaberColor;
        optional<UnityEngine::Color> ObstacleColor;
        optional<UnityEngine::Color> EnvironmentColor0;
        optional<UnityEngine::Color> EnvironmentColor1;
        optional<UnityEngine::Color> EnvironmentColorW;
        optional<UnityEngine::Color> EnvironmentColor0Boost;
        optional<UnityEngine::Color> EnvironmentColor1Boost;
        optional<UnityEngine::Color> EnvironmentColorWBoost;
        bool SupportsEnvironmentColorBoost = false;
        int EnvironmentEffectsFilterDefaultPreset = 0;
        int EnvironmentEffectsFilterExpertPlusPreset = 0;
        int EnvironmentEffectsFilterPreset = 0;
        bool NoTextsAndHuds = false;
        float SaberTrailIntensity = 0.0f;
        bool HideNoteSpawnEffect = false;
        bool ArcsHapticFeedback = false;
        int ArcVisibility = 0;
        optional<ReplayControllerOffsets> ControllerOffsets;
    };

    struct NoteEvent
    {
        NoteEvent();
        NoteEvent(NoteID TheNoteID, NoteEventType EventType, VRPosition CutPoint, VRPosition CutNormal, VRPosition SaberDirection, int SaberType, bool DirectionOK,
                  float SaberSpeed, float CutAngle, float CutDistanceToCenter, float CutDirectionDeviation, float BeforeCutRating, float AfterCutRating,
                  float Time, float UnityTimescale, float TimeSyncTimescale);
        NoteEvent(NoteID TheNoteID, NoteEventType EventType, VRPosition CutPoint, VRPosition CutNormal, VRPosition SaberDirection, int SaberType, bool DirectionOK,
                  float SaberSpeed, float CutAngle, float CutDistanceToCenter, float CutDirectionDeviation, float BeforeCutRating, float AfterCutRating,
                  float Time, float UnityTimescale, float TimeSyncTimescale, float TimeDeviation, VRRotation WorldRotation, VRRotation InverseWorldRotation,
                  VRRotation NoteRotation, VRPosition NotePosition);
        NoteID TheNoteID;
        NoteEventType EventType;
        VRPosition CutPoint;
        VRPosition CutNormal;
        VRPosition SaberDirection;
        int SaberType;
        bool DirectionOK;
        float SaberSpeed;
        float CutAngle;
        float CutDistanceToCenter;
        float CutDirectionDeviation;
        float BeforeCutRating;
        float AfterCutRating;
        float Time;
        float UnityTimescale;
        float TimeSyncTimescale;

        // stored but not replayed
        std::optional<float> TimeDeviation;
        std::optional<VRRotation> WorldRotation;
        std::optional<VRRotation> InverseWorldRotation;
        std::optional<VRRotation> NoteRotation;
        std::optional<VRPosition> NotePosition;
        void Mirror();
    };

    struct ReplayFile
    {
        ReplayFile();
        ReplayFile(std::shared_ptr<Metadata> metadata, vector<VRPoseGroup> poseKeyframes, vector<HeightEvent> heightKeyframes, vector<NoteEvent> noteKeyframes,
                   vector<ScoreEvent> scoreKeyframes, vector<ComboEvent> comboKeyframes, vector<MultiplierEvent> multiplierKeyframes,
                   vector<EnergyEvent> energyKeyframes);
        shared_ptr<Metadata> metadata;
        vector<VRPoseGroup> poseKeyframes;
        vector<HeightEvent> heightKeyframes;
        vector<NoteEvent> noteKeyframes;
        vector<ScoreEvent> scoreKeyframes;
        vector<ComboEvent> comboKeyframes;
        vector<MultiplierEvent> multiplierKeyframes;
        vector<EnergyEvent> energyKeyframes;
        vector<PauseEvent> pauseKeyframes;
        vector<WallEvent> wallKeyframes;
        vector<char> hsvConfig;
        void Mirror();
    };
    UnityEngine::Vector3 VRVector3(VRPosition pose);
    UnityEngine::Quaternion VRQuaternion(VRRotation rotation);

} // namespace SnoreSaber::Data::Private
