#pragma once

#include "Features/Live/Ludus/Packets/LudusPacketDispatcher.hpp"
#include "logging.hpp"

#include <cctype>
#include <optional>
#include <string>

namespace SnoreSaber::Features::Live::Ludus::Packets::Handlers
{
    template <typename TSession>
    class ErrorEnvelopeHandler final : public ILudusEnvelopeHandler<TSession>
    {
      public:
        Protocol::LudusEnvelopeType Type() const override
        {
            return Protocol::LudusEnvelopeType::Error;
        }

        void Handle(TSession& session, const Protocol::DecodedLudusEnvelope& envelope) override
        {
            std::string status = "Ludus error " + envelope.ErrorCode + ": " + envelope.ErrorMessage;
            if (IsHiddenClientStatus(envelope))
            {
                ::SnoreSaber::Logging::Logger.debug("{}", status);
                return;
            }

            if (EqualsIgnoreCase(envelope.ErrorCode, "auth_failed"))
            {
                session.NotifyStatusChanged(status);
                WARN("{}", status);
                if (envelope.Retryable)
                {
                    if (session.RequestAuthenticationRefresh())
                    {
                        session.ScheduleReconnect("authentication failed", 0.5f);
                    }
                    else
                    {
                        session.ScheduleReconnect("authentication failed with a fresh game session", std::nullopt);
                    }
                    return;
                }

                session.Disconnect();
                return;
            }

            if (EqualsIgnoreCase(envelope.ErrorCode, "denied_mods"))
            {
                session.RejectPendingTournamentJoin(envelope.ErrorMessage);
                session.NotifyStatusChanged(envelope.ErrorMessage);
                WARN("{}", status);
                session.CloseTournamentRoom();
                return;
            }

            if (IsUnavailableTournamentRoom(envelope, session))
            {
                session.NotifyStatusChanged("Room closed.");
                WARN("{}", status);
                session.CloseTournamentRoom();
                return;
            }

            session.NotifyStatusChanged(status);
            WARN("{}", status);
        }

      private:
        static bool IsHiddenClientStatus(const Protocol::DecodedLudusEnvelope& envelope)
        {
            return EqualsIgnoreCase(envelope.ErrorCode, "packet_rejected");
        }

        static bool IsUnavailableTournamentRoom(const Protocol::DecodedLudusEnvelope& envelope, TSession& session)
        {
            if (session.RoomContext() != ::SnoreSaber::Live::V1::LudusRoomContextType::Tournament)
            {
                return false;
            }

            std::string error = ToLower(envelope.ErrorCode + " " + envelope.ErrorMessage);
            bool roomError = Contains(error, "room") || Contains(error, "match");
            if (!roomError)
            {
                return false;
            }

            return Contains(error, "closed") ||
                   Contains(error, "gone") ||
                   Contains(error, "missing") ||
                   Contains(error, "not found") ||
                   Contains(error, "not_found") ||
                   Contains(error, "not-in") ||
                   Contains(error, "not_in") ||
                   Contains(error, "unavailable");
        }

        static bool EqualsIgnoreCase(const std::string& left, const std::string& right)
        {
            return ToLower(left) == ToLower(right);
        }

        static std::string ToLower(std::string value)
        {
            for (auto& c : value)
            {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            return value;
        }

        static bool Contains(const std::string& haystack, const std::string& needle)
        {
            return haystack.find(needle) != std::string::npos;
        }
    };
}
