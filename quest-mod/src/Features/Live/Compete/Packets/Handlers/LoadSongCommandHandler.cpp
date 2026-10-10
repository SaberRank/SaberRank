#include "Features/Live/Compete/Packets/Handlers/LoadSongCommandHandler.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LiveSongCommand;
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::LudusDownloadState;
    using ::SnoreSaber::Live::V1::ServerCommand;

    namespace
    {
        bool EqualsIgnoreCase(const std::string& left, const std::string& right)
        {
            return std::equal(left.begin(), left.end(), right.begin(), right.end(), [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
        }

        bool MatchesSong(const Domain::CompeteSongSelection& selection, const LiveSongCommand& song)
        {
            return EqualsIgnoreCase(selection.mapHash, song.Hash);
        }
    }

    LudusCommandType LoadSongCommandHandler::Type() const
    {
        return LudusCommandType::LoadSong;
    }

    void LoadSongCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        // session is app-scoped in the session service, safe to capture across the worker
        Utils::Async::Run([&session, song = command.Song] {
            try
            {
                EnsureSongReady(session, song);
            }
            catch (const OperationCanceledException&)
            {
            }
            catch (const std::exception& ex)
            {
                // fire-and-forget fault log, PC RunTask parity
                ERROR("Ludus: Server command task failed: {}", ex.what());
            }
        });
    }

    bool LoadSongCommandHandler::EnsureSongReady(ILudusServerCommandSession& session, const std::optional<LiveSongCommand>& song)
    {
        auto room = session.TournamentRoom();
        if (!room || !song)
        {
            return false;
        }

        if (room->song && room->song->beatmapLevel && MatchesSong(*room->song, *song))
        {
            return true;
        }

        auto cancellationToken = session.ConnectionCancellationToken();
        auto installed = session.SongService()->ResolveInstalled(&*song, cancellationToken);
        if (installed)
        {
            room = session.TournamentRoom();
            if (!room)
            {
                return false;
            }

            auto updated = std::make_shared<Domain::CompeteRoom>(room->WithSong(installed));
            session.SetTournamentRoom(updated);
            session.NotifyRoomUpdated(updated);
            session.SendDownloadState(LudusDownloadState::Downloaded);
            return true;
        }

        auto preview = session.SongService()->CreatePreview(&*song, cancellationToken);
        room = session.TournamentRoom();
        if (!room)
        {
            return false;
        }

        auto downloading = std::make_shared<Domain::CompeteRoom>(room->WithSongStatus(preview ? preview : room->song, "Downloading map..."));
        session.SetTournamentRoom(downloading);
        session.NotifyRoomUpdated(downloading);
        session.SendDownloadState(LudusDownloadState::Downloading);

        try
        {
            auto resolved = session.SongService()->ResolveOrDownload(&*song, cancellationToken);
            if (!resolved || !resolved->beatmapLevel)
            {
                throw std::runtime_error("SongCore could not resolve the downloaded song");
            }

            room = session.TournamentRoom();
            if (!room)
            {
                return false;
            }

            auto updated = std::make_shared<Domain::CompeteRoom>(room->WithSong(resolved));
            session.SetTournamentRoom(updated);
            session.NotifyRoomUpdated(updated);
            session.SendDownloadState(LudusDownloadState::Downloaded);
            return true;
        }
        catch (const std::exception& ex)
        {
            // PC catches OperationCanceled here too (it derives from Exception)
            WARN("Failed to load live room song: {}", ex.what());
            room = session.TournamentRoom();
            if (!room)
            {
                return false;
            }

            auto failed = std::make_shared<Domain::CompeteRoom>(room->WithSongStatus(preview ? preview : room->song, "Map download failed."));
            session.SetTournamentRoom(failed);
            session.NotifyRoomUpdated(failed);
            session.SendDownloadState(LudusDownloadState::Error, ex.what());
            return false;
        }
    }

    std::optional<LiveSongCommand> LoadSongCommandHandler::SongCommandFromSelection(const std::shared_ptr<Domain::CompeteSongSelection>& song)
    {
        if (!song || song->mapHash.empty())
        {
            return std::nullopt;
        }

        return LiveSongCommand{
            .Hash = song->mapHash,
            .Difficulty = song->difficulty,
            .Characteristic = song->characteristic};
    }
}
