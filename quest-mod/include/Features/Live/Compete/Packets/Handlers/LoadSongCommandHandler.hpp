#pragma once

#include "Features/Live/Compete/Packets/LudusServerCommandDispatcher.hpp"

#include <memory>
#include <optional>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    class LoadSongCommandHandler final : public ILudusServerCommandHandler
    {
      public:
        ::SnoreSaber::Live::V1::LudusCommandType Type() const override;
        void Handle(ILudusServerCommandSession& session, const ::SnoreSaber::Live::V1::ServerCommand& command) override;

        // blocking; run on a worker thread
        static bool EnsureSongReady(ILudusServerCommandSession& session, const std::optional<::SnoreSaber::Live::V1::LiveSongCommand>& song);
        static std::optional<::SnoreSaber::Live::V1::LiveSongCommand> SongCommandFromSelection(const std::shared_ptr<Domain::CompeteSongSelection>& song);
    };
}
