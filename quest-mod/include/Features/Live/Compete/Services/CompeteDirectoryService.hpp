#pragma once

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompetePlayer.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/Domain/CompeteSongSelection.hpp"
#include "Features/Live/Compete/Domain/CompeteTournament.hpp"
#include "Features/Players/Domain/GameSession.hpp"
#include "Features/Players/Services/GameSessionService.hpp"

#include <custom-types/shared/macros.hpp>

#include <memory>
#include <string>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::Services, CompeteDirectoryService, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_CTOR(ctor, Players::Services::GameSessionService* gameSessionService);

public:
    // blocking; call off the main thread
    std::vector<Domain::CompeteTournament> GetActiveTournaments(const CancellationToken& cancellationToken);
    std::vector<Domain::CompeteRoom> GetJoinableRooms(const std::string& tournamentId, const CancellationToken& cancellationToken);
    Domain::CompeteRoom GetRoom(const std::string& tournamentId, const std::string& matchId, const CancellationToken& cancellationToken);
    Domain::CompeteRoom GetRoomByInviteCode(const std::string& inviteCode, const CancellationToken& cancellationToken);

private:
    Core::Api::SnoreSaberApiClient _apiClient;

    Data::GameSession GetSession(const CancellationToken& cancellationToken);
    Domain::CompeteRoom ToDomain(const Core::Api::Generated::LivePlayerRoomDetails& room, const CancellationToken& cancellationToken);
    Domain::CompetePlayer ToDomain(const Core::Api::Generated::LivePlayerRoomDetailsMembersItem& member);
    std::shared_ptr<Domain::CompeteSongSelection> ToSong(const Core::Api::Generated::LivePlayerRoomDetailsSelectedSong& song, const CancellationToken& cancellationToken);
    std::string FetchSongStars(const Core::Api::Generated::LivePlayerRoomDetailsSelectedSong& song, const CancellationToken& cancellationToken);
};
