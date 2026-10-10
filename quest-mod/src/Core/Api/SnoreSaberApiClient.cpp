#include "Core/Api/SnoreSaberApiClient.hpp"

#include "Core/Api/UploadTrust/UploadTrustHeaderBuilder.hpp"
#include "static.hpp"

#include <algorithm>
#include <cctype>
#include <future>
#include <paper2_scotland2/shared/string_convert.hpp>
#include <sstream>
#include <utility>

namespace SnoreSaber::Core::Api
{
    using SnoreSaber::Data::LeaderboardQuery;
    using SnoreSaber::Data::LeaderboardQueryScope;
    using SnoreSaber::Data::GameAuthenticationRequest;
    using SnoreSaber::Data::GameSession;
    using SnoreSaber::Data::PlayerListQuery;
    using SnoreSaber::Data::PlayerQueryScope;

    namespace
    {
        using namespace SnoreSaber::Core::Api::Generated;

        int ToInt(double value)
        {
            return static_cast<int>(value);
        }

        long ToLong(const std::string& value)
        {
            try
            {
                return std::stol(value);
            }
            catch (...)
            {
                return 0;
            }
        }

        std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
            return value;
        }

        bool StatusEquals(const std::string& value, const std::string& expected)
        {
            return ToLower(value) == ToLower(expected);
        }

        std::string JoinModifiers(const std::vector<std::string>& modifiers)
        {
            std::ostringstream result;
            for (size_t i = 0; i < modifiers.size(); ++i)
            {
                if (i > 0)
                    result << ",";
                result << modifiers[i];
            }
            return result.str();
        }

        std::u16string ToPlayerName(const std::string& playerNameInGame, const std::string& name)
        {
            return Paper::StringConvert::from_utf8(playerNameInGame.empty() ? name : playerNameInGame);
        }

        RequestHeaders ToHeaders(const GameSession* session)
        {
            RequestHeaders headers;
            if (!session || !session->IsAuthenticated())
                return headers;

            headers.sessionId = session->sessionId;
            headers.sessionKey = session->sessionKey;
            return headers;
        }

        GameAuthenticateRequest ToGeneratedAuthRequest(const GameAuthenticationRequest& request)
        {
            GameAuthenticateRequest generated;
            generated.At = request.authType;
            generated.PlayerId = request.playerId;
            generated.Nonce = request.nonce;
            generated.Friends = request.friendIds;
            generated.Name = request.playerName;
            return generated;
        }

        std::optional<Pivot> GetLeaderboardPivot(LeaderboardQueryScope scope)
        {
            switch (scope)
            {
                case LeaderboardQueryScope::AroundPlayer:
                    return Pivot::Player;
                case LeaderboardQueryScope::Friends:
                    return Pivot::Friends;
                default:
                    return std::nullopt;
            }
        }

        std::optional<std::string> GetLeaderboardScope(const LeaderboardQuery& query)
        {
            switch (query.scope)
            {
                case LeaderboardQueryScope::Country:
                    return "country";
                case LeaderboardQueryScope::Region:
                    return "region";
                case LeaderboardQueryScope::Countries:
                    return query.countries;
                default:
                    return std::nullopt;
            }
        }

        std::optional<Pivot> GetPlayerPivot(PlayerQueryScope scope)
        {
            switch (scope)
            {
                case PlayerQueryScope::AroundPlayer:
                    return Pivot::Player;
                case PlayerQueryScope::Friends:
                    return Pivot::Friends;
                default:
                    return std::nullopt;
            }
        }

        std::optional<PlayerScope> GetPlayerScope(PlayerQueryScope scope)
        {
            switch (scope)
            {
                case PlayerQueryScope::Country:
                    return PlayerScope::Country;
                case PlayerQueryScope::Region:
                    return PlayerScope::Region;
                default:
                    return std::nullopt;
            }
        }

        std::optional<std::string> GetCountries(const PlayerListQuery& query)
        {
            return query.scope == PlayerQueryScope::Countries ? std::make_optional(query.countries) : std::nullopt;
        }

        std::optional<std::string> GetQueryFlag(bool enabled)
        {
            return enabled ? std::make_optional(std::string("true")) : std::nullopt;
        }

        bool IsNoPlayerScoreResponse(const LeaderboardQuery& query, const ApiException& exception)
        {
            if (query.scope != LeaderboardQueryScope::AroundPlayer || exception.statusCode != 404)
                return false;

            std::string errorText = ToLower(exception.response + " " + exception.message);
            return errorText.find("hasn't set a score") != std::string::npos;
        }

        Data::LeaderboardPlayer ToPlayer(const LeaderboardScoresResponseDataItemPlayer& source)
        {
            Data::LeaderboardPlayer player;
            player.id = source.Id;
            player.name = ToPlayerName(source.PlayerNameInGame, source.Name);
            player.profilePicture = source.Avatar;
            player.country = source.Country;
            player.permissions = ToInt(source.Permissions);
            player.role = source.Role;
            return player;
        }

        Data::LeaderboardPlayer ToPlayer(const LeaderboardScoresResponsePlayerScorePlayer& source)
        {
            Data::LeaderboardPlayer player;
            player.id = source.Id;
            player.name = ToPlayerName(source.PlayerNameInGame, source.Name);
            player.profilePicture = source.Avatar;
            player.country = source.Country;
            player.permissions = ToInt(source.Permissions);
            player.role = source.Role;
            return player;
        }

        Data::Score ToScore(const LeaderboardScoresResponseDataItem& source)
        {
            Data::Score score;
            score.id = ToInt(source.Id);
            score.leaderboardPlayerInfo = ToPlayer(source.Player);
            score.rank = ToInt(source.Rank);
            score.baseScore = ToInt(source.UnmodifiedScore);
            score.modifiedScore = ToInt(source.ModifiedScore);
            score.pp = source.PP;
            score.weight = source.Weight;
            score.modifiers = JoinModifiers(source.Mods);
            score.multiplier = source.UnmodifiedScore == 0.0 ? 0.0 : source.ModifiedScore / source.UnmodifiedScore;
            score.badCuts = ToInt(source.BadCuts);
            score.missedNotes = ToInt(source.MissedNotes);
            score.maxCombo = ToInt(source.MaxCombo);
            score.fullCombo = source.FullCombo;
            score.hmd = source.LegacyHMDId.has_value() ? ToInt(source.LegacyHMDId.value()) : 0;
            score.deviceHmd = source.Device.HMD;
            score.deviceControllerLeft = source.Device.ControllerLeft;
            score.deviceControllerRight = source.Device.ControllerRight;
            score.personalBest = source.PersonalBest;
            score.playOutcome = source.PlayOutcome;
            score.playOutcomeTime = source.PlayOutcomeTime;
            score.hasReplay = source.HasReplay;
            score.timeSet = source.CreatedAt;
            return score;
        }

        Data::Score ToScore(const LeaderboardScoresResponsePlayerScore& source)
        {
            Data::Score score;
            score.id = ToInt(source.Id);
            score.leaderboardPlayerInfo = ToPlayer(source.Player);
            score.rank = ToInt(source.Rank);
            score.baseScore = ToInt(source.UnmodifiedScore);
            score.modifiedScore = ToInt(source.ModifiedScore);
            score.pp = source.PP;
            score.weight = source.Weight;
            score.modifiers = JoinModifiers(source.Mods);
            score.multiplier = source.UnmodifiedScore == 0.0 ? 0.0 : source.ModifiedScore / source.UnmodifiedScore;
            score.badCuts = ToInt(source.BadCuts);
            score.missedNotes = ToInt(source.MissedNotes);
            score.maxCombo = ToInt(source.MaxCombo);
            score.fullCombo = source.FullCombo;
            score.hmd = source.LegacyHMDId.has_value() ? ToInt(source.LegacyHMDId.value()) : 0;
            score.deviceHmd = source.Device.HMD;
            score.deviceControllerLeft = source.Device.ControllerLeft;
            score.deviceControllerRight = source.Device.ControllerRight;
            score.personalBest = source.PersonalBest;
            score.playOutcome = source.PlayOutcome;
            score.playOutcomeTime = source.PlayOutcomeTime;
            score.hasReplay = source.HasReplay;
            score.timeSet = source.CreatedAt;
            return score;
        }

        template <typename TMetadata>
        Paging::PageMetadata ToPageMetadata(const TMetadata& source)
        {
            Paging::PageMetadata metadata;
            metadata.page = ToInt(source.Page);
            metadata.itemsPerPage = ToInt(source.ItemsPerPage);
            metadata.totalItems = ToInt(source.TotalItems);
            metadata.totalPages = ToInt(source.TotalPages);
            return metadata;
        }

        bool HasPlayerScore(const LeaderboardScoresResponsePlayerScore& source)
        {
            return source.Id > 0.0 || source.Rank > 0.0;
        }

        bool IsSuccessfulUploadCreatedResponse(const ApiException& exception)
        {
            if (exception.statusCode != 201)
                return false;

            try
            {
                return GameUploadResponse::FromJson(exception.response).Success;
            }
            catch (...)
            {
                return false;
            }
        }

        Data::LeaderboardStatus ToLeaderboardStatus(const LeaderboardResponse& source)
        {
            if (StatusEquals(source.Realm.LeaderboardStatus, "ranked"))
                return Data::LeaderboardStatus::Ranked;
            if (StatusEquals(source.Realm.LeaderboardStatus, "qualified"))
                return Data::LeaderboardStatus::Qualified;
            if (StatusEquals(source.Realm.LeaderboardStatus, "loved"))
                return Data::LeaderboardStatus::Loved;
            return Data::LeaderboardStatus::Unranked;
        }

        Data::LeaderboardDetails ToLeaderboardDetails(const LeaderboardResponse& source)
        {
            Data::LeaderboardDetails details;
            details.id = ToInt(source.Id);
            details.songHash = source.Map.Hash;
            details.songName = source.Map.SongName;
            details.songSubName = source.Map.SongSubName;
            details.songAuthorName = source.Map.SongAuthorName;
            details.levelAuthorName = source.Map.LevelAuthorName;
            details.coverImage = source.Map.CoverUrl;
            details.difficulty = ToInt(source.Difficulty.Difficulty);
            details.difficultyRaw = source.Difficulty.RawDifficulty;
            details.gameMode = source.Difficulty.GameMode;
            details.maxScore = ToInt(source.MaxScore);
            details.plays = ToInt(source.TotalScores);
            details.dailyPlays = ToInt(source.DailyScores);
            details.createdAt = source.CreatedAt;
            details.rankedAt = source.Realm.RankedAt;
            details.qualifiedAt = source.Realm.QualifiedAt;
            details.lovedAt = source.Realm.LovedAt;
            details.status = ToLeaderboardStatus(source);
            details.positiveModifiers = source.Realm.PositiveModifiers;
            details.stars = source.Realm.Stars;
            details.realmId = ToInt(source.Realm.RealmId);
            details.realmName = source.Realm.RealmName;
            return details;
        }

        Data::ScoreStats ToScoreStats(const PlayerProfileResponseStats& source)
        {
            Data::ScoreStats stats;
            stats.totalScore = ToLong(source.TotalScore);
            stats.totalRankedScore = ToLong(source.TotalRankedScore);
            stats.averageRankedAccuracy = source.AverageAccuracy;
            stats.totalPlayCount = ToInt(source.TotalSubmittedPlays);
            stats.rankedPlayCount = ToInt(source.TotalPlayedRankedLeaderboards);
            stats.replaysWatched = ToInt(source.TotalReplayViews);
            return stats;
        }

        Data::ScoreStats ToScoreStats(const PlayerBasicProfileResponseStats& source)
        {
            Data::ScoreStats stats;
            stats.totalScore = ToLong(source.TotalScore);
            stats.totalRankedScore = ToLong(source.TotalRankedScore);
            stats.averageRankedAccuracy = source.AverageAccuracy;
            stats.totalPlayCount = ToInt(source.TotalSubmittedPlays);
            stats.rankedPlayCount = ToInt(source.TotalPlayedRankedLeaderboards);
            stats.replaysWatched = ToInt(source.TotalReplayViews);
            return stats;
        }

        Data::ScoreStats ToScoreStats(const PlayerListResponseDataItemStats& source)
        {
            Data::ScoreStats stats;
            stats.totalScore = ToLong(source.TotalScore);
            stats.totalRankedScore = ToLong(source.TotalRankedScore);
            stats.averageRankedAccuracy = source.AverageAccuracy;
            stats.totalPlayCount = ToInt(source.TotalSubmittedPlays);
            stats.rankedPlayCount = ToInt(source.TotalPlayedRankedLeaderboards);
            stats.replaysWatched = ToInt(source.TotalReplayViews);
            return stats;
        }

        Data::Player ToPlayer(const PlayerProfileResponse& source)
        {
            Data::Player player;
            player.id = source.Id;
            player.name = ToPlayerName(source.PlayerNameInGame, source.Name);
            player.plainName = source.Name;
            player.profilePicture = source.Avatar;
            player.country = source.Country;
            player.pp = source.Stats.TotalPP;
            player.rank = ToInt(source.Stats.Rank);
            player.countryRank = ToInt(source.Stats.CountryRank);
            player.role = source.Role.value_or("");
            player.scoreStats = ToScoreStats(source.Stats);
            player.permissions = ToInt(source.Permissions);
            player.banned = source.Banned;
            player.inactive = source.Inactive;

            for (const auto& badge : source.Badges)
            {
                Data::Badge dataBadge;
                dataBadge.description = badge.Description;
                dataBadge.image = badge.Image;
                player.badges.push_back(dataBadge);
            }

            return player;
        }

        Data::Player ToPlayer(const PlayerBasicProfileResponse& source)
        {
            Data::Player player;
            player.id = source.Id;
            player.name = ToPlayerName(source.PlayerNameInGame, source.Name);
            player.plainName = source.Name;
            player.profilePicture = source.Avatar;
            player.country = source.Country;
            player.pp = source.Stats.TotalPP;
            player.rank = ToInt(source.Stats.Rank);
            player.countryRank = ToInt(source.Stats.CountryRank);
            player.role = source.Role.value_or("");
            player.scoreStats = ToScoreStats(source.Stats);
            player.permissions = ToInt(source.Permissions);
            player.banned = source.Banned;
            player.inactive = source.Inactive;
            return player;
        }

        Data::Player ToPlayer(const PlayerListResponseDataItem& source)
        {
            Data::Player player;
            player.id = source.Id;
            player.name = ToPlayerName(source.PlayerNameInGame, source.Name);
            player.plainName = source.Name;
            player.profilePicture = source.Avatar;
            player.country = source.Country;
            player.pp = source.Stats.TotalPP;
            player.rank = ToInt(source.Stats.Rank);
            player.countryRank = ToInt(source.Stats.CountryRank);
            player.role = source.Role.value_or("");
            player.scoreStats = ToScoreStats(source.Stats);
            player.permissions = ToInt(source.Permissions);
            player.banned = source.Banned;
            player.inactive = source.Inactive;
            return player;
        }
    }

    SnoreSaberApiClient::SnoreSaberApiClient()
        : _generated(SnoreSaber::Static::BASE_URL), _uploadTrustClient()
    {
    }

    Data::GameAuthenticationResult SnoreSaberApiClient::AuthenticateGame(const Data::GameAuthenticationRequest& request)
    {
        try
        {
            GameAuthenticateRequest generatedRequest = ToGeneratedAuthRequest(request);
            _uploadTrustClient.ApplyAuthMetadata(generatedRequest);
            GameAuthenticateResponse response = _generated.AuthenticateGame(generatedRequest);
            if (response.SessionId.empty() || response.Key.empty())
            {
                return Data::GameAuthenticationResult::Failure("Failed to authenticate with SnoreSaber", SnoreSaberApiError::FromMessage("Missing game session"));
            }

            Data::GameSession session;
            session.playerId = request.playerId;
            session.playerName = request.playerName;
            session.sessionId = response.SessionId;
            session.sessionKey = response.Key;
            session.uploadTrust = CreateUploadTrustSession(response);
            return Data::GameAuthenticationResult::Success(std::move(session));
        }
        catch (const ApiException& exception)
        {
            SnoreSaberApiError error;
            error.statusCode = exception.statusCode;
            error.message = exception.message;
            error.rawBody = exception.response;
            return Data::GameAuthenticationResult::Failure(exception.message, error);
        }
        catch (const std::exception& exception)
        {
            return Data::GameAuthenticationResult::Failure(exception.what(), SnoreSaberApiError::FromMessage(exception.what()));
        }
    }

    std::optional<UploadTrust::UploadTrustSession> SnoreSaberApiClient::CreateUploadTrustSession(const GameAuthenticateResponse& response) const
    {
        return _uploadTrustClient.CreateSession(response);
    }

    Data::Private::ScoreUploadResult SnoreSaberApiClient::UploadScore(const Data::GameSession& session, const std::string& uploadData, const std::string& uploadVersionHash, const std::vector<char>& replay)
    {
        if (!session.IsAuthenticated())
            return Data::Private::ScoreUploadResult::Failure("SnoreSaber is not authenticated", SnoreSaberApiError::FromMessage("Missing game session"));
        if (!session.UsesUploadProtocolV2())
            return Data::Private::ScoreUploadResult::Failure("SnoreSaber upload trust is unavailable", SnoreSaberApiError::FromMessage("Current game session does not include v2 upload trust"));
        if (replay.empty())
            return Data::Private::ScoreUploadResult::Failure("Failed to serialize replay", SnoreSaberApiError::FromMessage("Replay payload was empty"));

        UploadTrust::UploadTrustHeaders headers;
        try
        {
            headers = UploadTrust::UploadTrustHeaderBuilder::BuildUploadHeaders(session.sessionId,
                                                                                session.playerId,
                                                                                uploadVersionHash,
                                                                                uploadData,
                                                                                replay,
                                                                                session.uploadTrust.value());
        }
        catch (const std::exception& exception)
        {
            return Data::Private::ScoreUploadResult::Failure(exception.what(), SnoreSaberApiError::FromMessage(exception.what()));
        }

        try
        {
            GameUploadResponse response = _generated.UploadScore(session.sessionKey,
                                                                 session.sessionId,
                                                                 uploadData,
                                                                 replay,
                                                                 headers.uploadVersionHash,
                                                                 headers.uploadSignature,
                                                                 headers.replaySha256,
                                                                 headers.uploadNonce,
                                                                 headers.uploadTimestamp,
                                                                 headers.clientBuildId,
                                                                 headers.uploadProtocol);

            return response.Success
                ? Data::Private::ScoreUploadResult::Success()
                : Data::Private::ScoreUploadResult::Failure("Failed to upload score");
        }
        catch (const ApiException& exception)
        {
            if (IsSuccessfulUploadCreatedResponse(exception))
                return Data::Private::ScoreUploadResult::Success();

            SnoreSaberApiError error;
            error.statusCode = exception.statusCode;
            error.message = exception.message;
            error.rawBody = exception.response;
            return Data::Private::ScoreUploadResult::Failure(exception.message, error);
        }
        catch (const std::exception& exception)
        {
            return Data::Private::ScoreUploadResult::Failure(exception.what(), SnoreSaberApiError::FromMessage(exception.what()));
        }
    }

    Data::LeaderboardSnapshot SnoreSaberApiClient::GetLeaderboard(const Data::LeaderboardQuery& query, const Data::GameSession* session)
    {
        auto detailsFuture = std::async(std::launch::async, [this, query] {
            return _generated.GetLeaderboard(query.songHash, query.gameMode, query.difficulty, query.realmId);
        });
        auto scoresFuture = std::async(std::launch::async, [this, query, session] {
            return _generated.GetLeaderboardScores(query.songHash,
                                                   query.gameMode,
                                                   query.difficulty,
                                                   query.page,
                                                   query.limit,
                                                   GetLeaderboardPivot(query.scope),
                                                   GetLeaderboardScope(query),
                                                   GetQueryFlag(query.hideNoArrows),
                                                   query.realmId,
                                                   GetQueryFlag(session && session->IsAuthenticated()),
                                                   ToHeaders(session));
        });

        LeaderboardScoresResponse scoresResponse;
        bool hasScores = true;
        try
        {
            scoresResponse = scoresFuture.get();
        }
        catch (const ApiException& exception)
        {
            if (!IsNoPlayerScoreResponse(query, exception))
                throw;

            hasScores = false;
        }

        Data::LeaderboardSnapshot leaderboard;
        leaderboard.leaderboard = ToLeaderboardDetails(detailsFuture.get());

        if (!hasScores)
            return leaderboard;

        for (const auto& score : scoresResponse.Data)
            leaderboard.scores.items.push_back(ToScore(score));
        leaderboard.scores.metadata = ToPageMetadata(scoresResponse.Metadata);

        if (HasPlayerScore(scoresResponse.PlayerScore))
            leaderboard.playerScore = ToScore(scoresResponse.PlayerScore);

        return leaderboard;
    }

    Paging::PagedResult<Data::Player> SnoreSaberApiClient::GetPlayers(const Data::PlayerListQuery& query, const Data::GameSession* session)
    {
        auto response = _generated.GetPlayers(query.page,
                                              query.limit,
                                              GetCountries(query),
                                              GetPlayerScope(query.scope),
                                              query.realmId,
                                              GetPlayerPivot(query.scope),
                                              ToHeaders(session));

        Paging::PagedResult<Data::Player> players;
        for (const auto& player : response.Data)
            players.items.push_back(ToPlayer(player));
        players.metadata = ToPageMetadata(response.Metadata);
        return players;
    }

    Data::Player SnoreSaberApiClient::GetPlayerProfile(const std::string& playerId, bool full, std::optional<int> realmId)
    {
        if (full)
            return ToPlayer(_generated.GetPlayer(playerId, realmId));

        return ToPlayer(_generated.GetPlayerBasic(playerId, realmId));
    }

    Generated::MapDetailsResponse SnoreSaberApiClient::GetMapById(int mapId)
    {
        return _generated.GetMapById(mapId);
    }

    Generated::MapDetailsResponse SnoreSaberApiClient::GetMapByHash(const std::string& hash)
    {
        return _generated.GetMapByHash(hash);
    }

    std::vector<Generated::LivePlayerTournamentSummary> SnoreSaberApiClient::ListLivePlayerTournaments(const Data::GameSession& session)
    {
        return _generated.ListLivePlayerTournaments(ToHeaders(&session));
    }

    std::vector<Generated::LivePlayerRoomSummary> SnoreSaberApiClient::ListLivePlayerRooms(const std::string& tournamentId, const Data::GameSession& session)
    {
        return _generated.ListLivePlayerRooms(tournamentId, ToHeaders(&session));
    }

    Generated::LivePlayerRoomDetails SnoreSaberApiClient::GetLivePlayerRoom(const std::string& tournamentId, const std::string& matchId, const Data::GameSession& session)
    {
        return _generated.GetLivePlayerRoom(tournamentId, matchId, ToHeaders(&session));
    }

    Generated::LivePlayerRoomDetails SnoreSaberApiClient::GetLivePlayerRoomByInviteCode(const std::string& inviteCode, const Data::GameSession& session)
    {
        return _generated.GetLivePlayerRoomByInviteCode(inviteCode, ToHeaders(&session));
    }

    std::vector<char> SnoreSaberApiClient::DownloadReplay(int scoreId)
    {
        return _generated.DownloadReplay(scoreId);
    }
}
