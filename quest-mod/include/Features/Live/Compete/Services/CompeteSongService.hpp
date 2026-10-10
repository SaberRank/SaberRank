#pragma once

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Core/BeatSaver/BeatSaverService.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteSongSelection.hpp"
#include "Features/Live/Protocol/Generated/Commands.hpp"

#include <custom-types/shared/macros.hpp>

#include <memory>
#include <optional>
#include <string>

namespace SnoreSaber::Features::Live::Compete::Services
{
    // merged snoresaber/beatsaver metadata describing a song before it is installed
    struct LiveSongDetails
    {
        std::string Hash;
        std::string Name;
        std::string Mapper;
        std::string Difficulty;
        std::string Characteristic;
        std::string CoverUrl;
        std::string DownloadUrl;
        std::string Duration{"--"};
        std::string Bpm{"--"};
        std::string Nps{"--"};
        std::string Notes{"--"};
        std::string Obstacles{"--"};
        std::string Bombs{"--"};
        std::string Njs{"--"};
        std::string JumpDistance{"--"};
        std::string Stars{"--"};
    };
}

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::Services, CompeteSongService, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::BeatSaver::BeatSaverService*, _beatSaver);
    DECLARE_CTOR(ctor, Core::BeatSaver::BeatSaverService* beatSaver);

public:
    // blocking; call off the main thread
    std::shared_ptr<Domain::CompeteSongSelection> ResolveOrDownload(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const CancellationToken& cancellationToken);
    std::shared_ptr<Domain::CompeteSongSelection> ResolveInstalled(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const CancellationToken& cancellationToken);
    std::shared_ptr<Domain::CompeteSongSelection> CreatePreview(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const CancellationToken& cancellationToken);

private:
    Core::Api::SnoreSaberApiClient _apiClient;

    std::shared_ptr<Domain::CompeteSongSelection> ResolveInstalled(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const std::optional<LiveSongDetails>& scoreSaberDetails, const CancellationToken& cancellationToken);
    std::optional<LiveSongDetails> TryFetchSnoreSaberSongDetails(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const CancellationToken& cancellationToken);
    std::optional<Core::BeatSaver::BeatSaverMap> TryFetchBeatSaverMap(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const CancellationToken& cancellationToken);
    LiveSongDetails BuildBeatSaverSongDetails(const ::SnoreSaber::Live::V1::LiveSongCommand* song, const Core::BeatSaver::BeatSaverMap* map, const Core::BeatSaver::BeatSaverVersion* version);
    void DownloadAndRefresh(const std::string& hash, const Core::BeatSaver::BeatSaverVersion* version, const CancellationToken& cancellationToken);
    void DownloadAndRefreshCore(const std::string& hash, const Core::BeatSaver::BeatSaverVersion* version, const CancellationToken& cancellationToken);
};
