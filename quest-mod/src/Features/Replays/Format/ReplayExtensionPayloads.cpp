#include "Features/Replays/Format/ReplayExtensionPayloads.hpp"

#include <cstring>
#include <sstream>

using namespace std;

namespace SnoreSaber::Data::Private::ReplayExtensionPayloads
{
    namespace
    {
        int WriteInt(int value, stringstream& outputStream)
        {
            outputStream.write(reinterpret_cast<const char*>(&value), sizeof(int));
            return 4;
        }

        int WriteFloat(float value, stringstream& outputStream)
        {
            outputStream.write(reinterpret_cast<const char*>(&value), sizeof(float));
            return 4;
        }

        int WriteBool(bool value, stringstream& outputStream)
        {
            char byte = value ? 1 : 0;
            outputStream.write(&byte, 1);
            return 1;
        }

        int WriteLong(int64_t value, stringstream& outputStream)
        {
            outputStream.write(reinterpret_cast<const char*>(&value), sizeof(int64_t));
            return 8;
        }

        int WriteString(const string& value, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteInt((int)value.size(), outputStream);
            outputStream.write(value.data(), value.size());
            bytesWritten += value.size();
            return bytesWritten;
        }

        int WriteColor(const optional<UnityEngine::Color>& color, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteBool(color.has_value(), outputStream);
            if (color.has_value())
            {
                bytesWritten += WriteFloat(color->r, outputStream);
                bytesWritten += WriteFloat(color->g, outputStream);
                bytesWritten += WriteFloat(color->b, outputStream);
                bytesWritten += WriteFloat(color->a, outputStream);
            }
            return bytesWritten;
        }

        int WriteVRPosition(const VRPosition& position, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteFloat(position.X, outputStream);
            bytesWritten += WriteFloat(position.Y, outputStream);
            bytesWritten += WriteFloat(position.Z, outputStream);
            return bytesWritten;
        }

        int WritePlaySettings(const Metadata& metadata, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteFloat(metadata.SongSpeed, outputStream);
            bytesWritten += WriteFloat(metadata.JumpDistance, outputStream);
            bytesWritten += WriteColor(metadata.LeftSaberColor, outputStream);
            bytesWritten += WriteColor(metadata.RightSaberColor, outputStream);
            bytesWritten += WriteColor(metadata.ObstacleColor, outputStream);
            bytesWritten += WriteColor(metadata.EnvironmentColor0, outputStream);
            bytesWritten += WriteColor(metadata.EnvironmentColor1, outputStream);
            bytesWritten += WriteColor(metadata.EnvironmentColorW, outputStream);
            bytesWritten += WriteColor(metadata.EnvironmentColor0Boost, outputStream);
            bytesWritten += WriteColor(metadata.EnvironmentColor1Boost, outputStream);
            bytesWritten += WriteColor(metadata.EnvironmentColorWBoost, outputStream);
            bytesWritten += WriteBool(metadata.SupportsEnvironmentColorBoost, outputStream);
            bytesWritten += WriteString(metadata.Environment, outputStream);
            bytesWritten += WriteInt(metadata.EnvironmentEffectsFilterDefaultPreset, outputStream);
            bytesWritten += WriteInt(metadata.EnvironmentEffectsFilterExpertPlusPreset, outputStream);
            bytesWritten += WriteInt(metadata.EnvironmentEffectsFilterPreset, outputStream);
            bytesWritten += WriteBool(metadata.NoTextsAndHuds, outputStream);
            bytesWritten += WriteFloat(metadata.SaberTrailIntensity, outputStream);
            bytesWritten += WriteBool(metadata.HideNoteSpawnEffect, outputStream);
            bytesWritten += WriteBool(metadata.ArcsHapticFeedback, outputStream);
            bytesWritten += WriteInt(metadata.ArcVisibility, outputStream);
            return bytesWritten;
        }

        int WritePauseEvent(const PauseEvent& pauseEvent, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteFloat(pauseEvent.Time, outputStream);
            bytesWritten += WriteLong(pauseEvent.Duration, outputStream);
            bytesWritten += WriteLong(pauseEvent.UnixStartTime, outputStream);
            bytesWritten += WriteLong(pauseEvent.UnixEndTime, outputStream);
            return bytesWritten;
        }

        int WriteWallEvent(const WallEvent& wallEvent, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteFloat(wallEvent.Time, outputStream);
            bytesWritten += WriteFloat(wallEvent.ExitTime, outputStream);
            bytesWritten += WriteFloat(wallEvent.Energy, outputStream);
            bytesWritten += WriteFloat(wallEvent.ObstacleTime, outputStream);
            bytesWritten += WriteFloat(wallEvent.ObstacleDuration, outputStream);
            bytesWritten += WriteInt(wallEvent.LineIndex, outputStream);
            bytesWritten += WriteInt(wallEvent.LineLayer, outputStream);
            bytesWritten += WriteInt(wallEvent.Width, outputStream);
            bytesWritten += WriteInt(wallEvent.Height, outputStream);
            return bytesWritten;
        }

        int WriteControllerOffset(const optional<ReplayControllerOffset>& offset, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteBool(offset.has_value(), outputStream);
            if (offset.has_value())
            {
                bytesWritten += WriteVRPosition(offset->Position, outputStream);
                bytesWritten += WriteVRPosition(offset->Rotation, outputStream);
            }
            return bytesWritten;
        }

        int WriteControllerOffsets(const ReplayControllerOffsets& offsets, stringstream& outputStream)
        {
            int bytesWritten = 0;
            bytesWritten += WriteControllerOffset(offsets.Shared, outputStream);
            bytesWritten += WriteControllerOffset(offsets.Left, outputStream);
            bytesWritten += WriteControllerOffset(offsets.Right, outputStream);
            return bytesWritten;
        }

        template <typename T, typename WriteItem>
        int WriteList(const vector<T>& values, stringstream& outputStream, WriteItem writeItem)
        {
            int bytesWritten = 0;
            bytesWritten += WriteInt((int)values.size(), outputStream);
            for (const T& value : values)
            {
                bytesWritten += writeItem(value, outputStream);
            }
            return bytesWritten;
        }

        template <typename WritePayload>
        ReplayExtensionEntry CreateExtension(std::string_view id, int version, WritePayload writePayload)
        {
            stringstream stream;
            writePayload(stream);
            string payload = stream.str();
            return ReplayExtensionEntry {string(id), version, vector<char>(payload.begin(), payload.end())};
        }

        bool HasPlaySettings(const Metadata& metadata)
        {
            return metadata.HasPlaySettingsExtension ||
                   !metadata.Environment.empty() ||
                   metadata.SongSpeed > 0.0f ||
                   metadata.JumpDistance > 0.0f ||
                   metadata.LeftSaberColor.has_value() ||
                   metadata.RightSaberColor.has_value() ||
                   metadata.ObstacleColor.has_value() ||
                   metadata.EnvironmentColor0.has_value() ||
                   metadata.EnvironmentColor1.has_value() ||
                   metadata.EnvironmentColorW.has_value() ||
                   metadata.EnvironmentColor0Boost.has_value() ||
                   metadata.EnvironmentColor1Boost.has_value() ||
                   metadata.EnvironmentColorWBoost.has_value();
        }
    }

    bool HasFileExtensions(const ReplayFile& file)
    {
        return (file.metadata && (HasPlaySettings(*file.metadata) || file.metadata->ControllerOffsets.has_value())) ||
               !file.pauseKeyframes.empty() ||
               !file.wallKeyframes.empty() ||
               !file.hsvConfig.empty();
    }

    std::vector<ReplayExtensionEntry> CreateFileExtensions(const ReplayFile& file)
    {
        vector<ReplayExtensionEntry> entries;
        if (file.metadata)
        {
            entries = CreateStartExtensions(*file.metadata, file.hsvConfig);
        }
        if (!file.pauseKeyframes.empty())
        {
            entries.push_back(CreatePauseEvents(file.pauseKeyframes));
        }
        if (!file.wallKeyframes.empty())
        {
            entries.push_back(CreateWallEvents(file.wallKeyframes));
        }

        return entries;
    }

    std::vector<ReplayExtensionEntry> CreateStartExtensions(const Metadata& metadata, const std::vector<char>& hsvConfig)
    {
        vector<ReplayExtensionEntry> entries;
        if (HasPlaySettings(metadata))
        {
            entries.push_back(CreateExtension(PlaySettingsExtension, 1, [&metadata](stringstream& stream) {
                WritePlaySettings(metadata, stream);
            }));
        }
        if (metadata.ControllerOffsets.has_value())
        {
            entries.push_back(CreateExtension(ControllerOffsetsExtension, 1, [&metadata](stringstream& stream) {
                WriteControllerOffsets(metadata.ControllerOffsets.value(), stream);
            }));
        }
        if (!hsvConfig.empty())
        {
            entries.push_back(ReplayExtensionEntry {string(HsvConfigExtension), 1, hsvConfig});
        }

        return entries;
    }

    ReplayExtensionEntry CreatePauseEvents(const std::vector<PauseEvent>& pauseEvents)
    {
        return CreateExtension(PauseEventsExtension, 1, [&pauseEvents](stringstream& stream) {
            WriteList(pauseEvents, stream, WritePauseEvent);
        });
    }

    ReplayExtensionEntry CreateWallEvents(const std::vector<WallEvent>& wallEvents)
    {
        return CreateExtension(WallEventsExtension, 1, [&wallEvents](stringstream& stream) {
            WriteList(wallEvents, stream, WriteWallEvent);
        });
    }
} // namespace SnoreSaber::Data::Private::ReplayExtensionPayloads
