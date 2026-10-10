#pragma once

#include "Features/Live/Compete/Packets/ILudusServerCommandSession.hpp"
#include "Features/Live/Protocol/Generated/Commands.hpp"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace SnoreSaber::Features::Live::Compete::Packets
{
    struct ILudusServerCommandHandler
    {
        virtual ~ILudusServerCommandHandler() = default;
        virtual ::SnoreSaber::Live::V1::LudusCommandType Type() const = 0;
        virtual void Handle(ILudusServerCommandSession& session, const ::SnoreSaber::Live::V1::ServerCommand& command) = 0;
    };

    class LudusServerCommandDispatcher
    {
      public:
        static LudusServerCommandDispatcher CreateDefault();

        void Handle(ILudusServerCommandSession& session, const std::optional<::SnoreSaber::Live::V1::ServerCommand>& command);

      private:
        explicit LudusServerCommandDispatcher(std::vector<std::unique_ptr<ILudusServerCommandHandler>> handlers);

        std::unordered_map<::SnoreSaber::Live::V1::LudusCommandType, std::unique_ptr<ILudusServerCommandHandler>> _handlers;
    };
}
