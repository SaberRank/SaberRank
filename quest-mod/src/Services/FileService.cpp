#include <System/IO/Directory.hpp>
#include "Utils/StringUtils.hpp"
#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include "logging.hpp"
#include "static.hpp"
#include <exception>

namespace SnoreSaber::Services::FileService
{
    std::string SanitizeReplayFileNameComponent(std::string value, std::size_t maxLength = 155)
    {
        value = StringUtils::ReplaceInvalidChars(StringUtils::Truncate(value, maxLength, false));
        return value.empty() ? "_" : value;
    }

    void EnsurePaths()
    {
        try
        {
            if (!direxists(SnoreSaber::Static::DATA_DIR))
            {
                System::IO::Directory::CreateDirectory(SnoreSaber::Static::DATA_DIR);
            }
            if (!direxists(SnoreSaber::Static::REPLAY_DIR))
            {
                System::IO::Directory::CreateDirectory(SnoreSaber::Static::REPLAY_DIR);
            }
            if (!direxists(SnoreSaber::Static::REPLAY_TMP_DIR))
            {
                System::IO::Directory::CreateDirectory(SnoreSaber::Static::REPLAY_TMP_DIR);
            }
        }
        catch (const std::exception& exception)
        {
            ERROR("Failed to ensure SnoreSaber paths: {:s}", exception.what());
        }
    }

    std::string GetReplayFileName(std::string levelId, std::string difficultyName, std::string characteristic, std::string playerId, std::string songName)
    {
        levelId = SanitizeReplayFileNameComponent(levelId, 80);
        difficultyName = SanitizeReplayFileNameComponent(difficultyName, 40);
        characteristic = SanitizeReplayFileNameComponent(characteristic, 80);
        playerId = SanitizeReplayFileNameComponent(playerId, 80);
        songName = SanitizeReplayFileNameComponent(songName);
        std::string replayPath = playerId + "-" + songName + "-" + difficultyName + "-" + characteristic + "-" + levelId;
        return replayPath;
    }
} // namespace SnoreSaber::Services::FileService
