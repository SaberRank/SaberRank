using SnoreSaber.Core;
using SnoreSaber.Features.Live.Compete.Domain;
using SnoreSaber.Features.Live.Compete.Packets;
using SnoreSaber.Features.Live.Compete.Services;
using SnoreSaber.Live.V1;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace SnoreSaber.Features.Live.Compete.Packets.Handlers {
    internal sealed class StartMapCommandHandler : ILudusServerCommandHandler {
        public LudusCommandType Type => LudusCommandType.LudusCommandTypeStartMap;

        public void Handle(ILudusServerCommandSession session, ServerCommand command) {
            StartMap(session, command).RunTask();
        }

        private static async Task StartMap(ILudusServerCommandSession session, ServerCommand command) {
            CancellationToken cancellationToken = session.ConnectionCancellationToken;
            // A start command can race the room-join acknowledgement when the user enters from menus.
            // Wait briefly for the room context instead of silently dropping the command.
            DateTime roomWaitDeadline = DateTime.UtcNow.AddSeconds(8);
            try {
                while (!HasTournamentRoom(session, command.MatchId) && DateTime.UtcNow < roomWaitDeadline) {
                    cancellationToken.ThrowIfCancellationRequested();
                    await Task.Delay(100, cancellationToken);
                }
            } catch (OperationCanceledException) {
                return;
            }
            if (!HasTournamentRoom(session, command.MatchId)) {
                session.NotifyStatusChanged("Timed out waiting for the Ludus room before starting the map.");
                Plugin.Log.Warn($"Ludus: StartMap {command.MatchId} arrived before the room was ready.");
                return;
            }

            bool countdownBegun = false;
            try {
                if (session.TournamentRoom.Song == null || session.TournamentRoom.Song.BeatmapLevel == null) {
                    LiveSongCommand song = command.Song ?? LoadSongCommandHandler.SongCommandFromSelection(session.TournamentRoom.Song);
                    if (song != null) {
                        await LoadSongCommandHandler.EnsureSongReady(session, song);
                    }
                }

                if (session.TournamentRoom?.Song?.BeatmapLevel == null) {
                    throw new InvalidOperationException("Live room song is not ready");
                }

                CompeteRoom room = session.TournamentRoom;
                int delayMs = session.GameplayLauncher.StartDelayMs(command);
                cancellationToken = session.BeginMapStartCountdown(command.MatchId, delayMs, cancellationToken);
                countdownBegun = true;
                session.NotifyStatusChanged(delayMs > 0 ? "Map starting soon..." : "Starting map...");
                Plugin.Log.Info($"Ludus: Starting room map {room.Song.Name} for {room.Id}.");
                await session.GameplayLauncher.Start(room, delayMs, cancellationToken);
                if (!await session.GameplayLauncher.WaitForMapStartReady(room.Id, room.Song.MapHash, cancellationToken)) {
                    return;
                }

                session.SendPresence(LudusPlayState.LudusPlayStateInGame, LudusDownloadState.LudusDownloadStateNone, room.Song.MapHash);
            } catch (OperationCanceledException) {
            } catch (Exception ex) {
                session.NotifyStatusChanged($"Failed to start map: {ex.Message}");
                Plugin.Log.Warn($"Ludus: Failed to start map: {ex.Message}");
            } finally {
                if (countdownBegun) {
                    session.CompletePendingMapStart(command.MatchId, cancellationToken);
                }
            }
        }

        private static bool HasTournamentRoom(ILudusServerCommandSession session, string matchId) {
            return session.TournamentRoom != null && (string.IsNullOrEmpty(matchId) || string.Equals(matchId, session.TournamentRoom.Id, StringComparison.Ordinal) || string.Equals(matchId, session.TournamentRoom.TournamentId, StringComparison.Ordinal));
        }
    }
}
