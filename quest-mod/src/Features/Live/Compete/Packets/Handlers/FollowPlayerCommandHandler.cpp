#include "Features/Live/Compete/Packets/Handlers/FollowPlayerCommandHandler.hpp"

#include "logging.hpp"

#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>

#include <algorithm>

namespace SnoreSaber::Features::Live::Compete::Packets::Handlers
{
    using ::SnoreSaber::Live::V1::LudusCommandType;
    using ::SnoreSaber::Live::V1::ServerCommand;

    namespace
    {
        int ViewerCount(const ServerCommand& command)
        {
            if (command.PayloadJson.empty())
            {
                return 0;
            }

            rapidjson::Document document;
            document.Parse(command.PayloadJson.data(), command.PayloadJson.size());
            if (document.HasParseError() || !document.IsObject())
            {
                WARN("Ludus: Failed to read follow request payload.");
                return 0;
            }

            auto it = document.FindMember("viewerCount");
            if (it == document.MemberEnd())
            {
                return 0;
            }

            if (!it->value.IsInt())
            {
                WARN("Ludus: Failed to read follow request payload.");
                return 0;
            }

            return std::max(0, it->value.GetInt());
        }
    }

    LudusCommandType FollowPlayerCommandHandler::Type() const
    {
        return LudusCommandType::FollowPlayer;
    }

    void FollowPlayerCommandHandler::Handle(ILudusServerCommandSession& session, const ServerCommand& command)
    {
        session.NotifyPlayerFollowRequested(ViewerCount(command));
    }
}
