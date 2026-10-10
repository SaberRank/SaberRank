#include "Features/Replays/ReplayStorageService.hpp"

#include "Data/Private/Settings.hpp"
#include "Features/Leaderboards/Domain/SnoreSaberBeatmapKey.hpp"
#include "Features/Replays/ReplayLimits.hpp"
#include "Services/FileService.hpp"
#include "Utils/StringUtils.hpp"
#include "logging.hpp"
#include "static.hpp"
#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapDifficultySerializedMethods.hpp>
#include <fstream>

DEFINE_TYPE(SnoreSaber::ReplaySystem, ReplayStorageService);

namespace SnoreSaber::ReplaySystem
{
    namespace
    {
        std::string LegacyReplayPathFor(const std::string& replayFileName)
        {
            return SnoreSaber::Static::REPLAY_DIR + "/" + SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(replayFileName) + ".dat";
        }

        std::string LegacyReplayPathFor(const std::string& playerId, const std::string& songName, const std::string& songHash)
        {
            return SnoreSaber::Static::REPLAY_DIR + "/" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(playerId, 80) + "-" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(songName) + "-" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(songHash, 80) + ".dat";
        }

        std::string ReplayPathFor(const std::string& playerId, const std::string& songHash, GlobalNamespace::BeatmapKey beatmapKey)
        {
            std::string difficulty = GlobalNamespace::BeatmapDifficultySerializedMethods::SerializedName(beatmapKey.difficulty);
            std::string characteristic = beatmapKey.beatmapCharacteristic ? (std::string)beatmapKey.beatmapCharacteristic->serializedName : "Unknown";
            return SnoreSaber::Static::REPLAY_DIR + "/" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(playerId, 80) + "-" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(songHash, 80) + "-" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(difficulty, 40) + "-" +
                   SnoreSaber::Services::FileService::SanitizeReplayFileNameComponent(characteristic, 80) + ".dat";
        }

        std::vector<std::string> GetReplayPaths(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName)
        {
            std::vector<std::string> paths;
            if (!score.leaderboardPlayerInfo.id.has_value())
            {
                return paths;
            }

            std::string playerId = score.leaderboardPlayerInfo.id.value();
            std::string songHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::GetSongHash(beatmapKey);
            paths.push_back(ReplayPathFor(playerId, songHash, beatmapKey));

            if (!legacyReplayFileName.empty())
            {
                paths.push_back(LegacyReplayPathFor(legacyReplayFileName));
            }

            if (beatmapLevel)
            {
                std::string songName = StringUtils::ReplaceInvalidChars(StringUtils::Truncate((std::string)beatmapLevel->songName, 155, false));
                paths.push_back(LegacyReplayPathFor(playerId, songName, songHash));
            }

            return paths;
        }
    }

    void ReplayStorageService::ctor()
    {
        INVOKE_CTOR();
    }

    bool ReplayStorageService::LocalReplayExists(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName)
    {
        return !GetExistingReplayPath(beatmapLevel, beatmapKey, score, legacyReplayFileName).empty();
    }

    std::optional<std::vector<char>> ReplayStorageService::ReadLocalReplay(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName)
    {
        std::string replayPath = GetExistingReplayPath(beatmapLevel, beatmapKey, score, legacyReplayFileName);
        if (replayPath.empty())
        {
            return std::nullopt;
        }

        std::ifstream replayFile(replayPath, std::ios::binary | std::ios::ate);
        if (!replayFile.good())
        {
            ERROR("Failed to open local replay: {:s}", replayPath.c_str());
            return std::nullopt;
        }

        auto fileEnd = replayFile.tellg();
        if (fileEnd == std::streampos(-1))
        {
            ERROR("Failed to measure local replay: {:s}", replayPath.c_str());
            return std::nullopt;
        }

        std::streamoff fileSize = fileEnd;
        if (fileSize < 0 || static_cast<std::size_t>(fileSize) > ReplayLimits::MaxCompressedReplayBytes)
        {
            ERROR("Local replay is too large: {:s}", replayPath.c_str());
            return std::nullopt;
        }

        std::vector<char> replayData(static_cast<std::size_t>(fileSize));
        replayFile.seekg(0);
        if (!replayData.empty() && !replayFile.read(replayData.data(), replayData.size()))
        {
            ERROR("Failed to read local replay: {:s}", replayPath.c_str());
            return std::nullopt;
        }

        return replayData;
    }

    void ReplayStorageService::SaveLocalReplay(const std::string& playerId, GlobalNamespace::BeatmapKey beatmapKey, const std::vector<char>& replay)
    {
        if (!SnoreSaber::Data::Private::Settings::saveLocalReplays || replay.empty())
        {
            return;
        }

        if (replay.size() > ReplayLimits::MaxCompressedReplayBytes)
        {
            ERROR("Not saving oversized local replay: {} bytes", replay.size());
            return;
        }

        try
        {
            SnoreSaber::Services::FileService::EnsurePaths();
            std::string songHash = SnoreSaber::Utils::SnoreSaberBeatmapKey::GetSongHash(beatmapKey);
            std::ofstream file(ReplayPathFor(playerId, songHash, beatmapKey), std::ios::binary);
            file.write(replay.data(), replay.size());
        }
        catch (const std::exception& exception)
        {
            ERROR("Failed to write local replay: {:s}", exception.what());
        }
    }

    std::string ReplayStorageService::GetReplayPath(const std::string& playerId, const std::string& songHash, GlobalNamespace::BeatmapKey beatmapKey)
    {
        return ReplayPathFor(playerId, songHash, beatmapKey);
    }

    std::string ReplayStorageService::GetExistingReplayPath(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName)
    {
        for (const auto& replayPath : GetReplayPaths(beatmapLevel, beatmapKey, score, legacyReplayFileName))
        {
            if (fileexists(replayPath))
            {
                return replayPath;
            }
        }

        return "";
    }
}
