#include "Features/Replays/Format/ReplayReader.hpp"

#include "Features/Replays/ReplayLimits.hpp"
#include "Utils/lzma/lzma.hpp"

#include <optional>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Utils/Versions.hpp"
#include "logging.hpp"
#include "static.hpp"

using namespace std;

namespace SnoreSaber::Data::Private::ReplayReader
{
    Pointers ReadPointers(stringstream& inputStream);
    shared_ptr<Metadata> ReadMetadata(stringstream& inputStream, int offset);
    vector<VRPoseGroup> ReadPoseKeyframes(stringstream& inputStream, int offset);
    vector<HeightEvent> ReadHeightKeyframes(stringstream& inputStream, int offset);
    vector<NoteEvent> ReadNoteKeyframes(stringstream& inputStream, int offset);
    vector<NoteEvent> ReadNoteKeyframes_v3(stringstream& inputStream, int offset);
    vector<ScoreEvent> ReadScoreKeyframes(stringstream& inputStream, int offset);
    vector<ScoreEvent> ReadScoreKeyframes_v3(stringstream& inputStream, int offset);
    vector<ComboEvent> ReadComboKeyframes(stringstream& inputStream, int offset);
    vector<MultiplierEvent> ReadMultiplierKeyframes(stringstream& inputStream, int offset);
    vector<EnergyEvent> ReadEnergyKeyframes(stringstream& inputStream, int offset);
    VRPosition ReadVRPosition(stringstream& inputStream);
    VRRotation ReadVRRotation(stringstream& inputStream);
    VRPoseGroup ReadVRPoseGroup(stringstream& inputStream);
    VRPose ReadVRPose(stringstream& inputStream);
    HeightEvent ReadHeightEvent(stringstream& inputStream);
    NoteEvent ReadNoteEvent(stringstream& inputStream);
    NoteEvent ReadNoteEvent_v3(stringstream& inputStream);
    NoteID ReadNoteID(stringstream& inputStream);
    NoteID ReadNoteID_v3(stringstream& inputStream);
    ScoreEvent ReadScoreEvent(stringstream& inputStream);
    ScoreEvent ReadScoreEvent_v3(stringstream& inputStream);
    ComboEvent ReadComboEvent(stringstream& inputStream);
    MultiplierEvent ReadMultiplierEvent(stringstream& inputStream);
    EnergyEvent ReadEnergyEvent(stringstream& inputStream);
    int ReadInt(stringstream& inputStream);
    string ReadString(stringstream& inputStream);
    vector<string> ReadStringArray(stringstream& inputStream);
    float ReadFloat(stringstream& inputStream);
    bool ReadBool(stringstream& inputStream);
    int64_t ReadLong(stringstream& inputStream);
    optional<UnityEngine::Color> ReadColor(stringstream& inputStream);
    void ReadExtensions(stringstream& inputStream, ReplayFile& replay, int offset);
    void ReadPlaySettings(stringstream& inputStream, ReplayFile& replay);
    void ReadControllerOffsets(stringstream& inputStream, ReplayFile& replay);
    PauseEvent ReadPauseEvent(stringstream& inputStream);
    WallEvent ReadWallEvent(stringstream& inputStream);
    optional<ReplayControllerOffset> ReadControllerOffset(stringstream& inputStream);
    bool DecompressReplay(const vector<char>& replay, vector<char>& decompressed);

    namespace
    {
        constexpr int MaxReplayStringBytes = 1024 * 1024;
        constexpr int MaxReplayStringArrayItems = 256;
        constexpr int MaxReplayKeyframeCount = 2'000'000;

        constexpr int ExtensionMagic = 0x31585353; // SSX1
        constexpr int ExtensionTableVersion = 1;
        constexpr std::string_view PlaySettingsExtension = "snoresaber.play-settings";
        constexpr std::string_view PauseEventsExtension = "snoresaber.pause-events";
        constexpr std::string_view WallEventsExtension = "snoresaber.wall-events";
        constexpr std::string_view ControllerOffsetsExtension = "snoresaber.controller-offsets";
        constexpr std::string_view HsvConfigExtension = "snoresaber.hsv-config";

        std::streamoff StreamSize(stringstream& inputStream)
        {
            auto current = inputStream.tellg();
            inputStream.clear();
            inputStream.seekg(0, ios::end);
            auto size = inputStream.tellg();
            inputStream.clear();

            if (current != std::streampos(-1))
            {
                inputStream.seekg(current);
            }

            if (size == std::streampos(-1))
            {
                throw runtime_error("replay stream size is invalid");
            }

            return size;
        }

        void SeekToOffset(stringstream& inputStream, int offset)
        {
            if (offset < 0)
            {
                throw runtime_error("replay offset is negative");
            }

            auto size = StreamSize(inputStream);
            if (static_cast<std::streamoff>(offset) > size)
            {
                throw runtime_error("replay offset is outside the buffer");
            }

            inputStream.clear();
            inputStream.seekg(offset);
            if (!inputStream.good())
            {
                throw runtime_error("failed to seek replay stream");
            }
        }

        int ReadCount(stringstream& inputStream, const char* label)
        {
            int count = ReadInt(inputStream);
            if (count < 0 || count > MaxReplayKeyframeCount)
            {
                throw runtime_error(string("invalid replay ") + label + " count");
            }

            return count;
        }
    }

    shared_ptr<ReplayFile> Read(const vector<char> &replayData)
    {
        try
        {
            std::vector<char> decompressed;
            if(!DecompressReplay(replayData, decompressed)) {
                return nullptr;
            }
            if (decompressed.size() < sizeof(int) * 9)
            {
                ERROR("Replay data is missing pointer table");
                return nullptr;
            }

            stringstream inputStream;
            inputStream.write(decompressed.data(), decompressed.size());
            Pointers pointers = ReadPointers(inputStream);

            shared_ptr<Metadata> metadata = ReadMetadata(inputStream, pointers.metadata);

            INFO("Found replay with version {:s}", metadata->Version.str());
            shared_ptr<ReplayFile> replay;
            if (metadata->Version == version("2.0.0")) {
                replay = make_shared<ReplayFile>(metadata,
                                                 ReadPoseKeyframes(inputStream, pointers.poseKeyframes),
                                                 ReadHeightKeyframes(inputStream, pointers.heightKeyframes),
                                                 ReadNoteKeyframes(inputStream, pointers.noteKeyframes),
                                                 ReadScoreKeyframes(inputStream, pointers.scoreKeyframes),
                                                 ReadComboKeyframes(inputStream, pointers.comboKeyframes),
                                                 ReadMultiplierKeyframes(inputStream, pointers.multiplierKeyframes),
                                                 ReadEnergyKeyframes(inputStream, pointers.energyKeyframes));
            } else if (metadata->Version <= version("3.1.0")) {
                replay = make_shared<ReplayFile>(metadata,
                                                 ReadPoseKeyframes(inputStream, pointers.poseKeyframes),
                                                 ReadHeightKeyframes(inputStream, pointers.heightKeyframes),
                                                 ReadNoteKeyframes_v3(inputStream, pointers.noteKeyframes),
                                                 ReadScoreKeyframes_v3(inputStream, pointers.scoreKeyframes),
                                                 ReadComboKeyframes(inputStream, pointers.comboKeyframes),
                                                 ReadMultiplierKeyframes(inputStream, pointers.multiplierKeyframes),
                                                 ReadEnergyKeyframes(inputStream, pointers.energyKeyframes));
            } else {
                ERROR("Unknown replay version (potentially you need to update the SnoreSaber mod!");
                return nullptr;
            }

            ReadExtensions(inputStream, *replay, pointers.extensions);
            return replay;
        }
        catch (const exception& exception)
        {
            ERROR("Invalid replay data: {:s}", exception.what());
            return nullptr;
        }
    }

    Pointers ReadPointers(stringstream& inputStream)
    {
        SeekToOffset(inputStream, 0);
        return Pointers(ReadInt(inputStream), ReadInt(inputStream),
                        ReadInt(inputStream), ReadInt(inputStream),
                        ReadInt(inputStream), ReadInt(inputStream),
                        ReadInt(inputStream), ReadInt(inputStream),
                        ReadInt(inputStream));
    }

    shared_ptr<Metadata> ReadMetadata(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);

        version ver = version(ReadString(inputStream));

        INFO("found version {:s}", ver.str());

        if(ver < version("3.1.0")) {
            return make_shared<Metadata>(ver, ReadString(inputStream), ReadInt(inputStream), ReadString(inputStream),
                            ReadString(inputStream), ReadStringArray(inputStream), ReadFloat(inputStream), ReadBool(inputStream),
                            ReadFloat(inputStream), ReadFloat(inputStream), ReadVRPosition(inputStream), ReadFloat(inputStream),
                            nullopt, nullopt, nullopt);
        } else {
            return make_shared<Metadata>(ver, ReadString(inputStream), ReadInt(inputStream), ReadString(inputStream),
                            ReadString(inputStream), ReadStringArray(inputStream), ReadFloat(inputStream), ReadBool(inputStream),
                            ReadFloat(inputStream), ReadFloat(inputStream), ReadVRPosition(inputStream), ReadFloat(inputStream),
                            version(ReadString(inputStream)),
                            version(ReadString(inputStream)), ReadString(inputStream));
        }
    }

    vector<VRPoseGroup> ReadPoseKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "pose keyframe");
        vector<VRPoseGroup> poseKeyframes = vector<VRPoseGroup>();
        for (int i = 0; i < count; i++)
        {
            poseKeyframes.push_back(ReadVRPoseGroup(inputStream));
        }
        return poseKeyframes;
    }

    vector<HeightEvent> ReadHeightKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "height keyframe");
        vector<HeightEvent> heightKeyframes = vector<HeightEvent>();
        for (int i = 0; i < count; i++)
        {
            heightKeyframes.push_back(ReadHeightEvent(inputStream));
        }
        return heightKeyframes;
    }

    vector<NoteEvent> ReadNoteKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "note keyframe");
        vector<NoteEvent> noteKeyframes = vector<NoteEvent>();
        for (int i = 0; i < count; i++)
        {
            noteKeyframes.push_back(ReadNoteEvent(inputStream));
        }
        return noteKeyframes;
    }

    vector<NoteEvent> ReadNoteKeyframes_v3(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "note keyframe");
        vector<NoteEvent> noteKeyframes = vector<NoteEvent>();
        for (int i = 0; i < count; i++)
        {
            noteKeyframes.push_back(ReadNoteEvent_v3(inputStream));
        }
        return noteKeyframes;
    }

    vector<ScoreEvent> ReadScoreKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "score keyframe");
        vector<ScoreEvent> scoreKeyframes = vector<ScoreEvent>();
        for (int i = 0; i < count; i++)
        {
            scoreKeyframes.push_back(ReadScoreEvent(inputStream));
        }
        return scoreKeyframes;
    }

    vector<ScoreEvent> ReadScoreKeyframes_v3(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "score keyframe");
        vector<ScoreEvent> scoreKeyframes = vector<ScoreEvent>();
        for (int i = 0; i < count; i++)
        {
            scoreKeyframes.push_back(ReadScoreEvent_v3(inputStream));
        }
        return scoreKeyframes;
    }

    vector<ComboEvent> ReadComboKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "combo keyframe");
        vector<ComboEvent> comboKeyframes = vector<ComboEvent>();
        for (int i = 0; i < count; i++)
        {
            comboKeyframes.push_back(ReadComboEvent(inputStream));
        }
        return comboKeyframes;
    }

    vector<MultiplierEvent> ReadMultiplierKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "multiplier keyframe");
        vector<MultiplierEvent> multiplierKeyframes = vector<MultiplierEvent>();
        for (int i = 0; i < count; i++)
        {
            multiplierKeyframes.push_back(ReadMultiplierEvent(inputStream));
        }
        return multiplierKeyframes;
    }

    vector<EnergyEvent> ReadEnergyKeyframes(stringstream& inputStream, int offset)
    {
        SeekToOffset(inputStream, offset);
        int count = ReadCount(inputStream, "energy keyframe");
        vector<EnergyEvent> energyKeyframes = vector<EnergyEvent>();
        for (int i = 0; i < count; i++)
        {
            energyKeyframes.push_back(ReadEnergyEvent(inputStream));
        }
        return energyKeyframes;
    }

    VRPosition ReadVRPosition(stringstream& inputStream)
    {
        return VRPosition(ReadFloat(inputStream), ReadFloat(inputStream),
                          ReadFloat(inputStream));
    }

    VRRotation ReadVRRotation(stringstream& inputStream)
    {
        return VRRotation(ReadFloat(inputStream), ReadFloat(inputStream),
                          ReadFloat(inputStream), ReadFloat(inputStream));
    }

    VRPoseGroup ReadVRPoseGroup(stringstream& inputStream)
    {
        return VRPoseGroup(ReadVRPose(inputStream), ReadVRPose(inputStream), ReadVRPose(inputStream), ReadInt(inputStream), ReadFloat(inputStream));
    }

    VRPose ReadVRPose(stringstream& inputStream)
    {
        return VRPose(ReadVRPosition(inputStream), ReadVRRotation(inputStream));
    }

    HeightEvent ReadHeightEvent(stringstream& inputStream)
    {
        return HeightEvent(ReadFloat(inputStream), ReadFloat(inputStream));
    }

    NoteEvent ReadNoteEvent(stringstream& inputStream)
    {
        return NoteEvent(ReadNoteID(inputStream), (NoteEventType)ReadInt(inputStream), ReadVRPosition(inputStream), ReadVRPosition(inputStream),
                         ReadVRPosition(inputStream), ReadInt(inputStream), ReadBool(inputStream), ReadFloat(inputStream), ReadFloat(inputStream),
                         ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream),
                         ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream));
    }

    NoteEvent ReadNoteEvent_v3(stringstream& inputStream)
    {
        return NoteEvent(ReadNoteID_v3(inputStream), (NoteEventType)ReadInt(inputStream), ReadVRPosition(inputStream), ReadVRPosition(inputStream),
                         ReadVRPosition(inputStream), ReadInt(inputStream), ReadBool(inputStream), ReadFloat(inputStream), ReadFloat(inputStream),
                         ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream),
                         ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream), ReadFloat(inputStream),
                         ReadVRRotation(inputStream), ReadVRRotation(inputStream), ReadVRRotation(inputStream), ReadVRPosition(inputStream));
    }

    NoteID ReadNoteID(stringstream& inputStream)
    {
        return NoteID(ReadFloat(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadInt(inputStream));
    }

    NoteID ReadNoteID_v3(stringstream& inputStream)
    {
        return NoteID(ReadFloat(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadInt(inputStream), ReadFloat(inputStream));
    }

    ScoreEvent ReadScoreEvent(stringstream& inputStream)
    {
        return ScoreEvent(ReadInt(inputStream), ReadFloat(inputStream));
    }
    ScoreEvent ReadScoreEvent_v3(stringstream& inputStream)
    {
        return ScoreEvent(ReadInt(inputStream), ReadFloat(inputStream), ReadInt(inputStream));
    }

    ComboEvent ReadComboEvent(stringstream& inputStream)
    {
        return ComboEvent(ReadInt(inputStream), ReadFloat(inputStream));
    }

    MultiplierEvent ReadMultiplierEvent(stringstream& inputStream)
    {
        return MultiplierEvent(ReadInt(inputStream), ReadFloat(inputStream), ReadFloat(inputStream));
    }

    EnergyEvent ReadEnergyEvent(stringstream& inputStream)
    {
        return EnergyEvent(ReadFloat(inputStream), ReadFloat(inputStream));
    }

    void ReadExtensions(stringstream& inputStream, ReplayFile& replay, int offset)
    {
        if (offset <= 0)
        {
            return;
        }

        try
        {
            auto size = StreamSize(inputStream);
            if (static_cast<std::streamoff>(offset) >= size)
            {
                return;
            }

            SeekToOffset(inputStream, offset);
            if (ReadInt(inputStream) != ExtensionMagic)
            {
                return;
            }
            if (ReadInt(inputStream) != ExtensionTableVersion)
            {
                return;
            }

            int entryCount = ReadCount(inputStream, "extension entry");
            for (int i = 0; i < entryCount; i++)
            {
                string id = ReadString(inputStream);
                int extensionVersion = ReadInt(inputStream);
                int payloadLength = ReadInt(inputStream);
                std::streamoff payloadOffset = inputStream.tellg();
                if (payloadLength < 0 || payloadOffset < 0 || payloadOffset + payloadLength > size)
                {
                    throw runtime_error("replay extension payload is out of bounds");
                }

                if (id == PlaySettingsExtension && extensionVersion == 1)
                {
                    ReadPlaySettings(inputStream, replay);
                }
                else if (id == PauseEventsExtension && extensionVersion == 1)
                {
                    int count = ReadCount(inputStream, "pause keyframe");
                    replay.pauseKeyframes.clear();
                    for (int keyframe = 0; keyframe < count; keyframe++)
                    {
                        replay.pauseKeyframes.push_back(ReadPauseEvent(inputStream));
                    }
                }
                else if (id == WallEventsExtension && extensionVersion == 1)
                {
                    int count = ReadCount(inputStream, "wall keyframe");
                    replay.wallKeyframes.clear();
                    for (int keyframe = 0; keyframe < count; keyframe++)
                    {
                        replay.wallKeyframes.push_back(ReadWallEvent(inputStream));
                    }
                }
                else if (id == ControllerOffsetsExtension && extensionVersion == 1)
                {
                    ReadControllerOffsets(inputStream, replay);
                }
                else if (id == HsvConfigExtension && extensionVersion == 1)
                {
                    replay.hsvConfig.resize(payloadLength);
                    if (payloadLength > 0 && !inputStream.read(replay.hsvConfig.data(), payloadLength))
                    {
                        throw runtime_error("unexpected end of replay while reading hsv config");
                    }
                }

                SeekToOffset(inputStream, (int)(payloadOffset + payloadLength));
            }
        }
        catch (const exception& exception)
        {
            INFO("Ignoring replay extensions: {:s}", exception.what());
        }
    }

    void ReadPlaySettings(stringstream& inputStream, ReplayFile& replay)
    {
        auto& metadata = *replay.metadata;
        metadata.HasPlaySettingsExtension = true;
        metadata.SongSpeed = ReadFloat(inputStream);
        metadata.JumpDistance = ReadFloat(inputStream);
        metadata.LeftSaberColor = ReadColor(inputStream);
        metadata.RightSaberColor = ReadColor(inputStream);
        metadata.ObstacleColor = ReadColor(inputStream);
        metadata.EnvironmentColor0 = ReadColor(inputStream);
        metadata.EnvironmentColor1 = ReadColor(inputStream);
        metadata.EnvironmentColorW = ReadColor(inputStream);
        metadata.EnvironmentColor0Boost = ReadColor(inputStream);
        metadata.EnvironmentColor1Boost = ReadColor(inputStream);
        metadata.EnvironmentColorWBoost = ReadColor(inputStream);
        metadata.SupportsEnvironmentColorBoost = ReadBool(inputStream);
        string environment = ReadString(inputStream);
        if (!environment.empty())
        {
            metadata.Environment = environment;
        }
        metadata.EnvironmentEffectsFilterDefaultPreset = ReadInt(inputStream);
        metadata.EnvironmentEffectsFilterExpertPlusPreset = ReadInt(inputStream);
        metadata.EnvironmentEffectsFilterPreset = ReadInt(inputStream);
        metadata.NoTextsAndHuds = ReadBool(inputStream);
        metadata.SaberTrailIntensity = ReadFloat(inputStream);
        metadata.HideNoteSpawnEffect = ReadBool(inputStream);
        metadata.ArcsHapticFeedback = ReadBool(inputStream);
        metadata.ArcVisibility = ReadInt(inputStream);
    }

    void ReadControllerOffsets(stringstream& inputStream, ReplayFile& replay)
    {
        ReplayControllerOffsets offsets;
        offsets.Shared = ReadControllerOffset(inputStream);
        offsets.Left = ReadControllerOffset(inputStream);
        offsets.Right = ReadControllerOffset(inputStream);
        replay.metadata->ControllerOffsets = offsets;
    }

    PauseEvent ReadPauseEvent(stringstream& inputStream)
    {
        PauseEvent pauseEvent;
        pauseEvent.Time = ReadFloat(inputStream);
        pauseEvent.Duration = ReadLong(inputStream);
        pauseEvent.UnixStartTime = ReadLong(inputStream);
        pauseEvent.UnixEndTime = ReadLong(inputStream);
        return pauseEvent;
    }

    WallEvent ReadWallEvent(stringstream& inputStream)
    {
        WallEvent wallEvent;
        wallEvent.Time = ReadFloat(inputStream);
        wallEvent.ExitTime = ReadFloat(inputStream);
        wallEvent.Energy = ReadFloat(inputStream);
        wallEvent.ObstacleTime = ReadFloat(inputStream);
        wallEvent.ObstacleDuration = ReadFloat(inputStream);
        wallEvent.LineIndex = ReadInt(inputStream);
        wallEvent.LineLayer = ReadInt(inputStream);
        wallEvent.Width = ReadInt(inputStream);
        wallEvent.Height = ReadInt(inputStream);
        return wallEvent;
    }

    optional<ReplayControllerOffset> ReadControllerOffset(stringstream& inputStream)
    {
        if (!ReadBool(inputStream))
        {
            return nullopt;
        }

        ReplayControllerOffset offset;
        offset.Position = ReadVRPosition(inputStream);
        offset.Rotation = ReadVRPosition(inputStream);
        return offset;
    }

    // Primitives

    int ReadInt(stringstream& inputStream)
    {
        int value = 0;
        if (!inputStream.read((char*)&value, sizeof(int)))
        {
            throw runtime_error("unexpected end of replay while reading int");
        }
        return value;
    }

    std::string ReadString(stringstream& inputStream)
    {
        int signedLength = ReadInt(inputStream);
        if (signedLength < 0 || signedLength > MaxReplayStringBytes)
        {
            throw runtime_error("invalid replay string length");
        }

        size_t stringLength = (size_t)signedLength;
        std::string value;
        value.resize(stringLength);
        if (stringLength > 0 && !inputStream.read(value.data(), stringLength))
        {
            throw runtime_error("unexpected end of replay while reading string");
        }
        return value;
    }

    std::vector<string> ReadStringArray(stringstream& inputStream)
    {
        int length = ReadInt(inputStream);
        if (length < 0 || length > MaxReplayStringArrayItems)
        {
            throw runtime_error("invalid replay string array length");
        }

        std::vector<string> value = std::vector<string>();
        for (int i = 0; i < length; i++)
        {
            value.push_back(ReadString(inputStream));
        }
        return value;
    }

    float ReadFloat(stringstream& inputStream)
    {
        float value = 0;
        if (!inputStream.read((char*)&value, sizeof(float)))
        {
            throw runtime_error("unexpected end of replay while reading float");
        }
        return value;
    }

    int64_t ReadLong(stringstream& inputStream)
    {
        int64_t value = 0;
        if (!inputStream.read((char*)&value, sizeof(int64_t)))
        {
            throw runtime_error("unexpected end of replay while reading long");
        }
        return value;
    }

    optional<UnityEngine::Color> ReadColor(stringstream& inputStream)
    {
        if (!ReadBool(inputStream))
        {
            return nullopt;
        }

        float r = ReadFloat(inputStream);
        float g = ReadFloat(inputStream);
        float b = ReadFloat(inputStream);
        float a = ReadFloat(inputStream);
        return UnityEngine::Color(r, g, b, a);
    }

    bool ReadBool(stringstream& inputStream)
    {
        bool value = 0;
        if (!inputStream.read((char*)&value, sizeof(bool)))
        {
            throw runtime_error("unexpected end of replay while reading bool");
        }
        return value;
    }

    bool DecompressReplay(const std::vector<char> &replay, std::vector<char> &decompressed)
    {
        if (replay.size() < 4)
        {
            ERROR("Replay data is empty");
            return false;
        }

        if (replay.size() > SnoreSaber::ReplaySystem::ReplayLimits::MaxCompressedReplayBytes)
        {
            ERROR("Replay data is too large: {} bytes", replay.size());
            return false;
        }

        if(replay[0] == (char)93 && replay[1] == 0 && replay[2] == 0 && replay[3] == (char)128) {
            ERROR("Can't load legacy replays");
            return false; // legacy replay
        }

        string magic = "SnoreSaber Replay 👌🤠\r\n";
        if (replay.size() < magic.size() || string(replay.begin(), replay.begin() + magic.size()) != magic) {
            ERROR("Invalid magic bytes in replay");
            return false; // invalid magic
        }

        // remove magic bytes
        vector<char> compressedReplayBytes(replay.begin() + magic.size(), replay.end());

        bool result = LZMA::lzmaDecompress(compressedReplayBytes, decompressed, SnoreSaber::ReplaySystem::ReplayLimits::MaxDecompressedReplayBytes);

        if (!result) {
            ERROR("decompression failed or replay exceeded {} decompressed bytes", SnoreSaber::ReplaySystem::ReplayLimits::MaxDecompressedReplayBytes);
        }

        return result;
    }

} // namespace SnoreSaber::Data::Private::ReplayReader
