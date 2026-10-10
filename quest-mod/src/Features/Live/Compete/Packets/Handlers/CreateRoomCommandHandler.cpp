#include "Features/Live/Compete/Packets/Handlers/CreateRoomCommandHandler.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::ServerCommand;

    namespace
    {
        bool EqualsIgnoreCase(const std::string& left, const std::string& right)
        {
            return std::equal(left.begin(), left.end(), right.begin(), right.end(), [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
        }

        bool ShouldKeepCurrentSong(const std::shared_ptr<Domain::CompeteSongSelection>& current, const std::shared_ptr<Domain::CompeteSongSelection>& details)
        {
            if (!current)
            {
                return false;
            }

            if (!details)
            {
                return true;
            }

            if (!current->mapHash.empty() && !details->mapHash.empty())
            {
                return EqualsIgnoreCase(current->mapHash, details->mapHash);
            }

            return EqualsIgnoreCase(current->name, details->name) &&
                   EqualsIgnoreCase(current->difficulty, details->difficulty) &&
                   EqualsIgnoreCase(current->characteristic, details->characteristic);
        }

        Domain::CompeteRoom MergeRoomDetails(const Domain::CompeteRoom& details, const Domain::CompeteRoom& current)
        {
            std::unordered_map<std::string, const Domain::CompetePlayer*> livePlayers;
            for (const auto& player : current.players)
            {
                if (!player.playerId.empty())
                {
                    livePlayers[player.playerId] = &player;
                }
            }

            std::vector<Domain::CompetePlayer> players;
            players.reserve(details.players.size());
            for (const auto& player : details.players)
            {
                auto it = player.playerId.empty() ? livePlayers.end() : livePlayers.find(player.playerId);
                if (it != livePlayers.end())
                {
                    players.push_back(Domain::CompetePlayer{
                        .name = player.name,
                        .status = it->second->status,
                        .teamId = player.teamId,
                        .rank = player.rank,
                        .isLocalPlayer = player.isLocalPlayer,
                        .playerId = player.playerId,
                        .isBot = it->second->isBot,
                        .avatarUrl = player.avatarUrl});
                }
                else
                {
                    players.push_back(player);
                }
            }

            auto song = ShouldKeepCurrentSong(current.song, details.song) ? current.song : details.song;
            // shared_ptr identity stands in for the PC ReferenceEquals check
            std::string songStatus = song == current.song ? current.songStatus : details.songStatus;

            Domain::CompeteRoom room = details;
            room.song = std::move(song);
            room.songStatus = std::move(songStatus);
            room.players = std::move(players);
            room.localPlayerReady = current.localPlayerReady;
            return room;
        }

        void RefreshRoomDetails(ILudusServerCommandSession& session, const std::string& matchId)
        {
            auto currentRoom = session.TournamentRoom();
            if (!currentRoom)
            {
                return;
            }

            if (!matchId.empty() && matchId != currentRoom->id)
            {
                return;
            }

            try
            {
                auto details = session.DirectoryService()->GetRoom(currentRoom->tournamentId, currentRoom->id, session.ConnectionCancellationToken());
                auto latest = session.TournamentRoom();
                if (!latest)
                {
                    return;
                }

                auto merged = std::make_shared<Domain::CompeteRoom>(MergeRoomDetails(details, *latest));
                session.SetTournamentRoom(merged);
                session.NotifyRoomUpdated(merged);
            }
            catch (const OperationCanceledException&)
            {
            }
            catch (const std::exception& ex)
            {
                WARN("Failed to refresh live room details: {}", ex.what());
            }
        }
    }

    LudusCommandType CreateRoomCommandHandler::Type() const
    {
        return LudusCommandType::CreateRoom;
    }

    void CreateRoomCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        // session is app-scoped in the session service, safe to capture across the worker
        Utils::Async::Run([&session, matchId = command.MatchId] {
            RefreshRoomDetails(session, matchId);
        });
    }
}
