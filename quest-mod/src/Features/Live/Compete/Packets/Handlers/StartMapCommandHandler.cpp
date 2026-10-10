#include "Features/Live/Compete/Packets/Handlers/StartMapCommandHandler.hpp"

#include "Features/Live/Compete/Packets/Handlers/LoadSongCommandHandler.hpp"
#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <fmt/format.h>

#include <chrono>
#include <stdexcept>
#include <thread>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::LudusDownloadState;
    using ::SnoreSaber::Live::V1::LudusPlayState;
    using ::SnoreSaber::Live::V1::ServerCommand;

    namespace
    {
        bool HasTournamentRoom(ILudusServerCommandSession& session, const std::string& matchId)
        {
            auto room = session.TournamentRoom();
            return room && (matchId.empty() || matchId == room->id || matchId == room->tournamentId);
        }

        void StartMap(ILudusServerCommandSession& session, const ServerCommand& command)
        {
            auto cancellationToken = session.ConnectionCancellationToken();
            // Starting from the menu can race the room-join acknowledgement. Give the
            // asynchronous room context a short window to arrive rather than dropping the command.
            const auto roomWaitDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(8);
            try
            {
                while (!HasTournamentRoom(session, command.MatchId) && std::chrono::steady_clock::now() < roomWaitDeadline)
                {
                    cancellationToken.ThrowIfCancellationRequested();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            catch (const OperationCanceledException&)
            {
                return;
            }
            if (!HasTournamentRoom(session, command.MatchId))
            {
                session.NotifyStatusChanged("Timed out waiting for the Ludus room before starting the map.");
                WARN("Ludus: StartMap {} arrived before the room was ready.", command.MatchId);
                return;
            }

            bool countdownBegun = false;
            try
            {
                auto room = session.TournamentRoom();
                if (room && (!room->song || !room->song->beatmapLevel))
                {
                    auto song = command.Song ? command.Song : LoadSongCommandHandler::SongCommandFromSelection(room->song);
                    if (song)
                    {
                        LoadSongCommandHandler::EnsureSongReady(session, song);
                    }
                }

                room = session.TournamentRoom();
                if (!room || !room->song || !room->song->beatmapLevel)
                {
                    throw std::runtime_error("Live room song is not ready");
                }

                int delayMs = session.GameplayLauncher()->StartDelayMs(&command);
                cancellationToken = session.BeginMapStartCountdown(command.MatchId, delayMs, cancellationToken);
                countdownBegun = true;
                session.NotifyStatusChanged(delayMs > 0 ? "Map starting soon..." : "Starting map...");
                INFO("Ludus: Starting room map {} for {}.", room->song->name, room->id);
                session.GameplayLauncher()->Start(room.get(), delayMs, cancellationToken);
                if (session.GameplayLauncher()->WaitForMapStartReady(room->id, room->song->mapHash, cancellationToken))
                {
                    session.SendPresence(LudusPlayState::InGame, LudusDownloadState::None, room->song->mapHash);
                }
            }
            catch (const OperationCanceledException&)
            {
            }
            catch (const std::exception& ex)
            {
                session.NotifyStatusChanged(fmt::format("Failed to start map: {}", ex.what()));
                WARN("Ludus: Failed to start map: {}", ex.what());
            }

            // PC finally block
            if (countdownBegun)
            {
                session.CompletePendingMapStart(command.MatchId, cancellationToken);
            }
        }
    }

    LudusCommandType StartMapCommandHandler::Type() const
    {
        return LudusCommandType::StartMap;
    }

    void StartMapCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        // session is app-scoped in the session service, safe to capture across the worker
        Utils::Async::Run([&session, command = command] {
            StartMap(session, command);
        });
    }
}
