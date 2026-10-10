#include "Features/Live/Compete/Packets/Handlers/RoomSnapshotEnvelopeHandler.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    namespace V1 = ::SnoreSaber::Live::V1;

    namespace
    {
        const V1::LiveMatchRoomState* FindSessionRoom(ILudusServerCommandSession& session, const std::vector<V1::LiveMatchRoomState>& rooms)
        {
            if (rooms.empty())
            {
                return nullptr;
            }

            auto tournamentRoom = session.TournamentRoom();
            if (tournamentRoom)
            {
                for (const auto& item : rooms)
                {
                    if (item.MatchId == tournamentRoom->id || item.RoomId == tournamentRoom->id)
                    {
                        return &item;
                    }
                }

                return nullptr;
            }

            auto localPlayerId = session.LocalPlayerId();
            if (!localPlayerId.empty())
            {
                for (const auto& item : rooms)
                {
                    if (item.MatchId == "player:" + localPlayerId ||
                        std::find(item.PlayerIds.begin(), item.PlayerIds.end(), localPlayerId) != item.PlayerIds.end())
                    {
                        return &item;
                    }
                }
            }

            return rooms.size() == 1 ? &rooms.front() : nullptr;
        }

        std::string FormatPlayerStatus(const V1::LiveRoomPlayerState& state)
        {
            if (state.ReadyState == V1::LudusReadyState::Ready)
            {
                return "Ready";
            }

            if (state.DownloadState == V1::LudusDownloadState::Downloading)
            {
                return "Downloading";
            }

            if (state.DownloadState == V1::LudusDownloadState::Error)
            {
                return "Download Error";
            }

            if (state.PlayState == V1::LudusPlayState::InGame)
            {
                return "In Game";
            }

            return "Waiting";
        }

        void ApplyRoomSnapshot(ILudusServerCommandSession& session, const std::vector<V1::LiveMatchRoomState>& rooms)
        {
            // pc distinguishes a null room list from an empty one; the decoded envelope always
            // carries a vector, and both end in a nullopt viewers update
            const V1::LiveMatchRoomState* room = FindSessionRoom(session, rooms);
            session.NotifyViewersUpdated(room ? std::optional(room->Viewers) : std::nullopt);
            auto tournamentRoom = session.TournamentRoom();
            if (!room || !tournamentRoom)
            {
                return;
            }

            std::unordered_map<std::string, const V1::LiveRoomPlayerState*> states;
            for (const auto& state : room->PlayerStates)
            {
                states[state.PlayerId] = &state;
            }

            auto localPlayerId = session.LocalPlayerId();
            std::vector<Domain::CompetePlayer> players;
            players.reserve(tournamentRoom->players.size());
            bool localReady = tournamentRoom->localPlayerReady;

            for (const auto& player : tournamentRoom->players)
            {
                auto it = states.find(player.playerId);
                if (it == states.end())
                {
                    players.push_back(player);
                    continue;
                }

                const auto& state = *it->second;
                bool isLocal = player.playerId == localPlayerId;
                if (isLocal)
                {
                    localReady = state.ReadyState == V1::LudusReadyState::Ready;
                }

                players.push_back(Domain::CompetePlayer{
                    .name = player.name,
                    .status = FormatPlayerStatus(state),
                    .teamId = player.teamId,
                    .rank = player.rank,
                    .isLocalPlayer = isLocal,
                    .playerId = player.playerId,
                    .isBot = state.IsBot,
                    .avatarUrl = player.avatarUrl});
            }

            auto updated = std::make_shared<Domain::CompeteRoom>(tournamentRoom->WithPlayers(std::move(players), localReady));
            updated->playerCount = std::max(tournamentRoom->playerCount, static_cast<int>(updated->players.size()));
            session.SetTournamentRoom(updated);
            session.NotifyRoomUpdated(updated);
        }
    }

    void RoomSnapshotEnvelopeHandler::Handle(Ludus::Services::ILudusSessionPacketContext& session, const Protocol::DecodedLudusEnvelope& envelope)
    {
        ApplyRoomSnapshot(*_commandSession, envelope.Rooms);
    }
}
