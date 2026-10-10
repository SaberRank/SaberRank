#pragma once

#include "Features/Live/Compete/Packets/LudusServerCommandDispatcher.hpp"

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    class CreateRoomCommandHandler final : public ILudusServerCommandHandler
    {
      public:
        ::SnoreSaber::Live::V1::LudusCommandType Type() const override;
        void Handle(ILudusServerCommandSession& session, const ::SnoreSaber::Live::V1::ServerCommand& command) override;
    };
}
