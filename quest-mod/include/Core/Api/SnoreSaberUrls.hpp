#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace SnoreSaber::Core::Api::SnoreSaberUrls
{
    inline constexpr std::string_view WebsiteBaseUrl = "https://snoresaber.vercel.app";
    inline constexpr std::string_view ApiBaseUrl = "https://snoresaber.vercel.app/api";
    inline constexpr std::string_view CdnBaseUrl = "https://snoresaber.vercel.app";
    inline constexpr std::string_view LudusUrl = "wss://live.snoresaber.com/v1/connect";

    inline std::string Leaderboard(int leaderboardId)
    {
        return std::string(WebsiteBaseUrl) + "/leaderboard/" + std::to_string(leaderboardId);
    }

    inline std::string Player(std::string_view playerId)
    {
        return std::string(WebsiteBaseUrl) + "/u/" + std::string(playerId);
    }

    inline std::string QuestPairing()
    {
        return std::string(WebsiteBaseUrl) + "/quest/pair";
    }

    inline std::string Flag(std::string_view country)
    {
        std::string normalized(country);
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
        return std::string(CdnBaseUrl) + "/flags/" + normalized + ".png";
    }
}
