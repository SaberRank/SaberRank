#include "Features/Replays/Services/ReplayQueryService.hpp"

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Features/Replays/ReplayLimits.hpp"
#include "logging.hpp"

DEFINE_TYPE(SnoreSaber::ReplaySystem::Services, ReplayQueryService);

namespace SnoreSaber::ReplaySystem::Services
{
    void ReplayQueryService::ctor(ReplayStorageService* replayStorageService)
    {
        INVOKE_CTOR();
        _replayStorageService = replayStorageService;
    }

    std::optional<std::vector<char>> ReplayQueryService::GetReplayData(GlobalNamespace::BeatmapLevel* beatmapLevel, GlobalNamespace::BeatmapKey beatmapKey, const SnoreSaber::Data::Score& score, const std::string& legacyReplayFileName)
    {
        try
        {
            Core::Api::SnoreSaberApiClient apiClient;
            std::vector<char> replayData = apiClient.DownloadReplay(score.id);
            if (!replayData.empty())
            {
                if (replayData.size() <= ReplayLimits::MaxCompressedReplayBytes)
                {
                    return replayData;
                }

                ERROR("Downloaded replay is too large: {} bytes", replayData.size());
            }
        }
        catch (const std::exception& exception)
        {
            INFO("Failed to download SnoreSaber replay, checking local fallback: {:s}", exception.what());
        }

        return _replayStorageService->ReadLocalReplay(beatmapLevel, beatmapKey, score, legacyReplayFileName);
    }
}
