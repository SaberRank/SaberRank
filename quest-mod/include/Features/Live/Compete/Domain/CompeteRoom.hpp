#pragma once

#include "Features/Live/Compete/Domain/CompetePlayer.hpp"
#include "Features/Live/Compete/Domain/CompetePlayerListMode.hpp"
#include "Features/Live/Compete/Domain/CompeteSongSelection.hpp"
#include "Features/Live/Compete/Domain/CompeteTeam.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SnoreSaber::Features::Live::Compete::Domain
{
    struct CompeteRoom
    {
        std::string id;
        std::string tournamentId;
        std::string name;
        std::string code;
        std::string round;
        std::string state;
        CompetePlayerListMode playerListMode = CompetePlayerListMode::Regular;
        std::vector<CompeteTeam> teams;
        // shared reference like the PC nullable Song; null means no selection
        std::shared_ptr<CompeteSongSelection> song;
        std::string songStatus;
        std::vector<CompetePlayer> players;
        bool localPlayerReady = false;
        int playerCount = 0;

        std::string DisplayName() const
        {
            return code.empty() ? name : name + " - " + code;
        }

        CompeteRoom WithPlayers(std::vector<CompetePlayer> newPlayers, bool newLocalPlayerReady) const
        {
            CompeteRoom room = *this;
            room.players = std::move(newPlayers);
            room.localPlayerReady = newLocalPlayerReady;
            return room;
        }

        CompeteRoom WithSong(std::shared_ptr<CompeteSongSelection> newSong) const
        {
            CompeteRoom room = *this;
            room.song = std::move(newSong);
            room.songStatus.clear();
            return room;
        }

        CompeteRoom WithSongStatus(std::shared_ptr<CompeteSongSelection> newSong, std::string newSongStatus) const
        {
            CompeteRoom room = *this;
            room.song = std::move(newSong);
            room.songStatus = std::move(newSongStatus);
            return room;
        }
    };
}
