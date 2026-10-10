#pragma once

#include <cstddef>
#include <string>

namespace SnoreSaber::Services::FileService
{
    void EnsurePaths();
    std::string SanitizeReplayFileNameComponent(std::string value, std::size_t maxLength = 155);
    std::string GetReplayFileName(std::string levelId, std::string difficultyName, std::string characteristic, std::string playerId, std::string songName);
}
