#include "Features/Live/Compete/Services/CompeteDirectoryService.hpp"

#include "logging.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <fmt/format.h>
#include <future>
#include <map>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteDirectoryService);

namespace SnoreSaber::Features::Live::Compete::Services
{
    using namespace SnoreSaber::Core::Api::Generated;

    namespace
    {
        bool IsBlank(const std::string& value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
        }

        std::string ToLower(std::string value)
        {
            for (auto& c : value)
            {
                c = std::tolower(static_cast<unsigned char>(c));
            }
            return value;
        }

        std::string RoomName(const std::string& matchId)
        {
            return matchId.empty() ? "Room" : matchId;
        }

        std::string FormatRoomState(const std::string& value)
        {
            std::string result;
            result.reserve(value.size());
            for (char c : value)
            {
                result.push_back(c == '_' ? ' ' : static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }

            bool newWord = true;
            for (auto& c : result)
            {
                if (c == ' ')
                {
                    newWord = true;
                    continue;
                }
                if (newWord)
                {
                    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                    newWord = false;
                }
            }
            return result;
        }

        std::string DisplaySongName(const std::string& songName, const std::string& songSubName)
        {
            return songSubName.empty() ? songName : songName + " " + songSubName;
        }

        std::string FormatDifficulty(const std::string& difficulty)
        {
            return ToLower(difficulty) == "expertplus" ? "Expert+" : difficulty;
        }

        std::string RawDifficultyPart(const std::string& rawDifficulty, int index)
        {
            if (IsBlank(rawDifficulty))
            {
                return "";
            }

            std::string trimmed = rawDifficulty;
            trimmed.erase(0, trimmed.find_first_not_of('_'));
            trimmed.erase(trimmed.find_last_not_of('_') + 1);

            std::vector<std::string> parts;
            size_t start = 0;
            while (true)
            {
                size_t split = trimmed.find('_', start);
                if (split == std::string::npos)
                {
                    parts.push_back(trimmed.substr(start));
                    break;
                }
                parts.push_back(trimmed.substr(start, split - start));
                start = split + 1;
            }

            return index >= 0 && index < static_cast<int>(parts.size()) ? parts[index] : "";
        }

        std::string DifficultyName(const MapDetailsResponseLeaderboardsItem& leaderboard)
        {
            std::string rawDifficulty = RawDifficultyPart(leaderboard.RawDifficulty, 0);
            if (!rawDifficulty.empty())
            {
                return rawDifficulty;
            }

            switch (static_cast<int>(leaderboard.Difficulty))
            {
                case 1: return "Easy";
                case 3: return "Normal";
                case 5: return "Hard";
                case 7: return "Expert";
                case 9: return "ExpertPlus";
                default: return "";
            }
        }

        std::string CharacteristicName(const MapDetailsResponseLeaderboardsItem& leaderboard)
        {
            std::string gameMode = leaderboard.GameMode;
            if (IsBlank(gameMode))
            {
                gameMode = RawDifficultyPart(leaderboard.RawDifficulty, 1);
            }

            if (IsBlank(gameMode))
            {
                return "";
            }

            return ToLower(gameMode.substr(0, 4)) == "solo" ? gameMode.substr(4) : gameMode;
        }

        std::string NormalizeDifficultyName(const std::string& difficulty)
        {
            std::string normalized;
            for (char c : difficulty)
            {
                if (c == '+')
                {
                    normalized += "Plus";
                }
                else if (c != '_' && c != ' ')
                {
                    normalized.push_back(c);
                }
            }
            return ToLower(normalized);
        }

        std::string NormalizeCharacteristicName(const std::string& characteristic)
        {
            std::string normalized;
            for (char c : characteristic)
            {
                if (c != '_' && c != ' ')
                {
                    normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                }
            }

            if (normalized.starts_with("solo"))
            {
                normalized = normalized.substr(4);
            }

            if (normalized == "90degree" || normalized == "generated90degree")
            {
                return "ninetydegree";
            }
            if (normalized == "360degree" || normalized == "generated360degree")
            {
                return "threesixtydegree";
            }
            return normalized;
        }

        const MapDetailsResponseLeaderboardsItem* SelectLeaderboard(const MapDetailsResponse& map, const LivePlayerRoomDetailsSelectedSong& song)
        {
            if (map.Leaderboards.empty())
            {
                return nullptr;
            }

            if (song.LeaderboardId.has_value())
            {
                for (const auto& leaderboard : map.Leaderboards)
                {
                    if (std::abs(leaderboard.Id - *song.LeaderboardId) < 0.1)
                    {
                        return &leaderboard;
                    }
                }
            }

            std::string difficulty = NormalizeDifficultyName(song.Difficulty);
            std::string characteristic = NormalizeCharacteristicName(song.Characteristic);
            std::vector<const MapDetailsResponseLeaderboardsItem*> leaderboards;
            for (const auto& leaderboard : map.Leaderboards)
            {
                if (difficulty.empty() || NormalizeDifficultyName(DifficultyName(leaderboard)) == difficulty)
                {
                    leaderboards.push_back(&leaderboard);
                }
            }

            if (leaderboards.empty())
            {
                for (const auto& leaderboard : map.Leaderboards)
                {
                    leaderboards.push_back(&leaderboard);
                }
            }

            if (!characteristic.empty())
            {
                for (const auto* leaderboard : leaderboards)
                {
                    if (NormalizeCharacteristicName(CharacteristicName(*leaderboard)) == characteristic)
                    {
                        return leaderboard;
                    }
                }
            }

            return leaderboards.front();
        }

        std::string TeamIdString(double teamId)
        {
            return fmt::format("{:.0f}", teamId);
        }

        std::string FormatMemberStatus(const LivePlayerRoomDetailsMembersItem& member)
        {
            if (!member.Connected)
            {
                return "Offline";
            }

            if (member.DownloadState == "DOWNLOADING")
            {
                return "Downloading";
            }

            if (member.DownloadState == "ERROR")
            {
                return "Download Error";
            }

            return FormatRoomState(member.PlayState);
        }

        std::vector<Domain::CompeteTeam> BuildTeams(const std::vector<LivePlayerRoomDetailsMembersItem>& members)
        {
            // ordered by team id; name is the first non-empty TeamName in the group
            std::map<double, std::string> groups;
            for (const auto& member : members)
            {
                if (!member.TeamId.has_value())
                {
                    continue;
                }

                auto [it, inserted] = groups.try_emplace(*member.TeamId, std::string());
                if (it->second.empty() && member.TeamName.has_value() && !member.TeamName->empty())
                {
                    it->second = *member.TeamName;
                }
            }

            std::vector<Domain::CompeteTeam> teams;
            teams.reserve(groups.size());
            int index = 0;
            for (const auto& [teamId, name] : groups)
            {
                teams.push_back({ TeamIdString(teamId), name.empty() ? fmt::format("Team {}", index + 1) : name });
                index++;
            }
            return teams;
        }

        std::string FormatStars(double value)
        {
            return value > 0.0 ? fmt::format("{:.2f}", value) : "--";
        }

        std::string FormatDuration(double seconds)
        {
            if (seconds <= 0)
            {
                return "--";
            }

            int totalSeconds = static_cast<int>(seconds);
            return fmt::format("{}:{:02}", totalSeconds / 60, totalSeconds % 60);
        }

        int ToInt(double value)
        {
            return static_cast<int>(std::llround(value));
        }

        bool HasSelectedSong(const LivePlayerRoomDetailsSelectedSong& song)
        {
            // the generated struct is a plain value; absent selection comes through empty
            return !song.MapHash.empty() || !song.SongName.empty();
        }
    }

    void CompeteDirectoryService::ctor(Players::Services::GameSessionService* gameSessionService)
    {
        INVOKE_CTOR();
        _gameSessionService = gameSessionService;
    }

    std::vector<Domain::CompeteTournament> CompeteDirectoryService::GetActiveTournaments(const CancellationToken& cancellationToken)
    {
        Data::GameSession session = GetSession(cancellationToken);
        auto tournaments = _apiClient.ListLivePlayerTournaments(session);
        INFO("Live tournaments loaded for player {}: {}", session.playerId, tournaments.size());

        std::vector<Domain::CompeteTournament> result;
        result.reserve(tournaments.size());
        for (const auto& tournament : tournaments)
        {
            result.push_back({
                tournament.TournamentId,
                !tournament.Name.empty()           ? tournament.Name
                : !tournament.TournamentId.empty() ? tournament.TournamentId
                                                   : "Tournament",
                tournament.RoomSummary,
            });
        }
        return result;
    }

    std::vector<Domain::CompeteRoom> CompeteDirectoryService::GetJoinableRooms(const std::string& tournamentId, const CancellationToken& cancellationToken)
    {
        Data::GameSession session = GetSession(cancellationToken);
        auto rooms = _apiClient.ListLivePlayerRooms(tournamentId, session);
        INFO("Live rooms loaded for player {} in {}: {}", session.playerId, tournamentId, rooms.size());

        std::vector<Domain::CompeteRoom> result;
        result.reserve(rooms.size());
        for (const auto& room : rooms)
        {
            result.push_back({
                .id = room.MatchId,
                .tournamentId = room.TournamentId,
                .name = RoomName(room.MatchId),
                .code = room.InviteCode,
                .round = room.MatchId,
                .state = FormatRoomState(room.State),
                .playerListMode = room.RosterMode == "TEAM" ? Domain::CompetePlayerListMode::Teams : Domain::CompetePlayerListMode::Regular,
                .playerCount = ToInt(room.PlayerCount),
            });
        }
        return result;
    }

    Domain::CompeteRoom CompeteDirectoryService::GetRoom(const std::string& tournamentId, const std::string& matchId, const CancellationToken& cancellationToken)
    {
        Data::GameSession session = GetSession(cancellationToken);
        auto room = _apiClient.GetLivePlayerRoom(tournamentId, matchId, session);
        return ToDomain(room, cancellationToken);
    }

    Domain::CompeteRoom CompeteDirectoryService::GetRoomByInviteCode(const std::string& inviteCode, const CancellationToken& cancellationToken)
    {
        Data::GameSession session = GetSession(cancellationToken);
        auto room = _apiClient.GetLivePlayerRoomByInviteCode(inviteCode, session);
        return ToDomain(room, cancellationToken);
    }

    Data::GameSession CompeteDirectoryService::GetSession(const CancellationToken& cancellationToken)
    {
        auto done = std::make_shared<std::promise<void>>();
        auto future = done->get_future();
        _gameSessionService->EnsureAuthenticated(false, [done](Players::Services::GameSessionService::LoginStatus) {
            done->set_value();
        });

        while (future.wait_for(std::chrono::milliseconds(25)) != std::future_status::ready)
        {
            cancellationToken.ThrowIfCancellationRequested();
        }

        cancellationToken.ThrowIfCancellationRequested();
        auto session = _gameSessionService->GetGameSession();
        if (!session.has_value())
        {
            throw std::runtime_error("SnoreSaber game session is not available");
        }
        return *session;
    }

    Domain::CompeteRoom CompeteDirectoryService::ToDomain(const LivePlayerRoomDetails& room, const CancellationToken& cancellationToken)
    {
        std::vector<Domain::CompetePlayer> players;
        for (const auto& member : room.Members)
        {
            if (member.Role == "PLAYER")
            {
                players.push_back(ToDomain(member));
            }
        }

        return {
            .id = room.MatchId,
            .tournamentId = room.TournamentId,
            .name = RoomName(room.MatchId),
            .code = room.InviteCode,
            .round = room.MatchId,
            .state = FormatRoomState(room.State),
            .playerListMode = room.RosterMode == "TEAM" ? Domain::CompetePlayerListMode::Teams : Domain::CompetePlayerListMode::Regular,
            .teams = BuildTeams(room.Members),
            .song = ToSong(room.SelectedSong, cancellationToken),
            .players = std::move(players),
            .playerCount = ToInt(room.PlayerCount),
        };
    }

    Domain::CompetePlayer CompeteDirectoryService::ToDomain(const LivePlayerRoomDetailsMembersItem& member)
    {
        std::string teamId = member.TeamId.has_value() ? TeamIdString(*member.TeamId) : "";
        std::string playerId = !member.PlayerId.empty() ? member.PlayerId : member.Player.Id;
        std::string localPlayerId = _gameSessionService->GetLocalPlayerId();
        bool isLocalPlayer = !localPlayerId.empty() && localPlayerId == playerId;

        return {
            .name = !member.Player.Name.empty() ? member.Player.Name
                    : !playerId.empty()         ? playerId
                                                : "Player",
            .status = FormatMemberStatus(member),
            .teamId = teamId,
            .isLocalPlayer = isLocalPlayer,
            .playerId = playerId,
            .isBot = member.IsBot,
            .avatarUrl = member.Player.Avatar,
        };
    }

    std::shared_ptr<Domain::CompeteSongSelection> CompeteDirectoryService::ToSong(const LivePlayerRoomDetailsSelectedSong& song, const CancellationToken& cancellationToken)
    {
        if (!HasSelectedSong(song))
        {
            return nullptr;
        }

        std::string stars = FetchSongStars(song, cancellationToken);
        auto selection = std::make_shared<Domain::CompeteSongSelection>();
        selection->name = DisplaySongName(song.SongName, song.SongSubName);
        selection->mapper = !song.LevelAuthorName.empty() ? song.LevelAuthorName
                            : !song.SongAuthorName.empty()  ? song.SongAuthorName
                                                            : "Unknown";
        selection->difficulty = FormatDifficulty(song.Difficulty);
        selection->characteristic = song.Characteristic;
        selection->coverSource = song.CoverUrl.value_or("");
        selection->duration = FormatDuration(song.DurationSeconds);
        selection->bpm = fmt::format("{:.0f}", song.Bpm);
        selection->nps = song.Nps <= 0 ? "--" : fmt::format("{:.2f}", song.Nps);
        selection->notes = "--";
        selection->obstacles = "--";
        selection->bombs = "--";
        selection->njs = "--";
        selection->jumpDistance = "--";
        selection->stars = stars;
        selection->mapHash = song.MapHash;
        selection->downloadUrl = song.DownloadUrl.value_or("");
        return selection;
    }

    std::string CompeteDirectoryService::FetchSongStars(const LivePlayerRoomDetailsSelectedSong& song, const CancellationToken& cancellationToken)
    {
        std::string hash = song.MapHash;
        hash.erase(0, hash.find_first_not_of(" \t\r\n"));
        hash.erase(hash.find_last_not_of(" \t\r\n") + 1);
        for (auto& c : hash)
        {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        if (hash.empty())
        {
            return "--";
        }

        try
        {
            cancellationToken.ThrowIfCancellationRequested();
            auto map = _apiClient.GetMapByHash(hash);
            const auto* leaderboard = SelectLeaderboard(map, song);
            return FormatStars(leaderboard ? leaderboard->Realm.Stars : 0.0);
        }
        catch (const OperationCanceledException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            WARN("Unable to fetch SnoreSaber live room song stars: {}", ex.what());
            return "--";
        }
    }
}
