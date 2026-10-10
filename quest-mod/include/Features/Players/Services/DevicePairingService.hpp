#pragma once

#include <functional>
#include <optional>
#include <string>

namespace SnoreSaber::Features::Players::Services::DevicePairing
{
    struct PairingResult
    {
        bool success = false;
        std::string message;
    };

    // strips spaces/hyphens and uppercases; nullopt when the result is not a valid 12-char code
    std::optional<std::string> NormalizeCode(const std::string& rawCode);

    // claims a one-time code from snoresaber.com/quest/pair, persists the quest credential,
    // then signs in through the normal auth flow; finished is invoked on the main thread
    void PairWithCode(const std::string& rawCode, std::function<void(PairingResult)> finished);

    // deletes the stored credential and resets the game session; main thread only
    void SignOut();
}
