#pragma once

#include "Core/Api/Generated/SnoreSaberApiGeneratedClient.hpp"
#include "Core/Api/Paging/PagedResult.hpp"
#include "Core/Api/UploadTrust/UploadTrustClient.hpp"
#include "Features/Leaderboards/Domain/LeaderboardQuery.hpp"
#include "Features/Leaderboards/Domain/LeaderboardSnapshot.hpp"
#include "Features/Players/Domain/GameAuthenticationRequest.hpp"
#include "Features/Players/Domain/GameAuthenticationResult.hpp"
#include "Features/Players/Domain/GameSession.hpp"
#include "Features/Players/Domain/Player.hpp"
#include "Features/Players/Domain/PlayerListQuery.hpp"
#include "Features/ScoreSubmission/Domain/ScoreUploadResult.hpp"

#include <optional>
#include <string>
#include <vector>

namespace SnoreSaber::Core::Api
{
    class SnoreSaberApiClient
    {
      public:
        SnoreSaberApiClient();

        Data::GameAuthenticationResult AuthenticateGame(const Data::GameAuthenticationRequest& request);
        Data::Private::ScoreUploadResult UploadScore(const Data::GameSession& session, const std::string& uploadData, const std::string& uploadVersionHash, const std::vector<char>& replay);
        Data::LeaderboardSnapshot GetLeaderboard(const Data::LeaderboardQuery& query, const Data::GameSession* session = nullptr);
        Paging::PagedResult<Data::Player> GetPlayers(const Data::PlayerListQuery& query, const Data::GameSession* session = nullptr);
        Data::Player GetPlayerProfile(const std::string& playerId, bool full, std::optional<int> realmId = std::nullopt);
        Generated::MapDetailsResponse GetMapById(int mapId);
        Generated::MapDetailsResponse GetMapByHash(const std::string& hash);
        std::vector<Generated::LivePlayerTournamentSummary> ListLivePlayerTournaments(const Data::GameSession& session);
        std::vector<Generated::LivePlayerRoomSummary> ListLivePlayerRooms(const std::string& tournamentId, const Data::GameSession& session);
        Generated::LivePlayerRoomDetails GetLivePlayerRoom(const std::string& tournamentId, const std::string& matchId, const Data::GameSession& session);
        Generated::LivePlayerRoomDetails GetLivePlayerRoomByInviteCode(const std::string& inviteCode, const Data::GameSession& session);
        std::vector<char> DownloadReplay(int scoreId);

      private:
        Generated::SnoreSaberApiGeneratedClient _generated;
        UploadTrust::UploadTrustClient _uploadTrustClient;

        std::optional<UploadTrust::UploadTrustSession> CreateUploadTrustSession(const Generated::GameAuthenticateResponse& response) const;
    };
}
