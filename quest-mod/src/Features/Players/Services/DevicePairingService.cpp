#include "Features/Players/Services/DevicePairingService.hpp"

#include "Core/Api/Generated/SnoreSaberApiGeneratedClient.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"
#include "static.hpp"

#include <beatsaber-hook/shared/utils/utils-functions.h>
#include <bsml/shared/Helpers/getters.hpp>

#include <cctype>
#include <filesystem>
#include <string_view>

namespace SnoreSaber::Features::Players::Services::DevicePairing
{
    namespace
    {
        // matches the backend device code format: no 0/1/I/O
        constexpr std::string_view CodeAlphabet = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";
        constexpr size_t CodeLength = 12;
        constexpr int DeviceCodeAuthType = 4;

        constexpr auto InvalidCodeMessage = "Invalid or expired code. Get a new code from snoresaber.com/quest/pair and try again";
        constexpr auto NetworkErrorMessage = "Couldn't reach SnoreSaber. Check your connection and try again";
        constexpr auto PersistFailedMessage = "Paired, but saving your sign-in failed. Please try again";
        constexpr auto SignInFailedMessage = "Paired, but signing in failed. Please restart your game";
    }

    std::optional<std::string> NormalizeCode(const std::string& rawCode)
    {
        std::string code;
        code.reserve(CodeLength);
        for (char character : rawCode)
        {
            if (std::isspace(static_cast<unsigned char>(character)) || character == '-')
            {
                continue;
            }

            code += static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }

        if (code.size() != CodeLength || code.find_first_not_of(CodeAlphabet) != std::string::npos)
        {
            return std::nullopt;
        }

        return code;
    }

    void PairWithCode(const std::string& rawCode, std::function<void(PairingResult)> finished)
    {
        std::function<void(bool, std::string)> finish = [finished = std::move(finished)](bool success, std::string message) {
            SnoreSaber::Utils::Async::Main([finished, success, message = std::move(message)] {
                if (finished)
                {
                    finished(PairingResult{success, message});
                }
            });
        };

        std::optional<std::string> code = NormalizeCode(rawCode);
        if (!code.has_value())
        {
            finish(false, "Pairing codes are 12 letters and digits. Check the code and try again");
            return;
        }

        SnoreSaber::Utils::Async::Run([code = code.value(), finish] {
            Core::Api::Generated::GameAuthenticateResponse response;
            try
            {
                Core::Api::Generated::SnoreSaberApiGeneratedClient apiClient(SnoreSaber::Static::BASE_URL);
                Core::Api::Generated::GameAuthenticateRequest request;
                request.At = DeviceCodeAuthType;
                // identity comes from the code itself; the field is only required by request validation
                request.PlayerId = "0";
                request.Nonce = code;
                response = apiClient.AuthenticateGame(request);
            }
            catch (const Core::Api::Generated::ApiException& exception)
            {
                ERROR("Device pairing claim failed ({:d}): {:s}", exception.statusCode, exception.message.c_str());
                finish(false, exception.statusCode == 403 ? InvalidCodeMessage : NetworkErrorMessage);
                return;
            }
            catch (const std::exception& exception)
            {
                ERROR("Device pairing claim failed: {:s}", exception.what());
                finish(false, NetworkErrorMessage);
                return;
            }

            if (response.QuestKey.empty() || response.PlayerId.empty())
            {
                ERROR("Device pairing claim succeeded but no quest credential was returned");
                finish(false, SignInFailedMessage);
                return;
            }

            std::string credential = response.QuestKey + ":" + response.PlayerId;
            std::error_code directoryError;
            std::filesystem::create_directories(SnoreSaber::Static::DATA_DIR, directoryError);
            if (!writefile(SnoreSaber::Static::STEAM_KEY_PATH, credential) || readfile(SnoreSaber::Static::STEAM_KEY_PATH) != credential)
            {
                ERROR("Failed to persist quest credential to {:s}", SnoreSaber::Static::STEAM_KEY_PATH.c_str());
                finish(false, PersistFailedMessage);
                return;
            }

            INFO("Device pairing claimed, signing in with stored credential");
            SnoreSaber::Utils::Async::Main([finish] {
                auto gameSessionService = BSML::Helpers::GetDiContainer()->TryResolve<GameSessionService*>();
                if (!gameSessionService)
                {
                    finish(false, SignInFailedMessage);
                    return;
                }

                gameSessionService->EnsureAuthenticated(true, [finish](GameSessionService::LoginStatus loginStatus) {
                    if (loginStatus == GameSessionService::LoginStatus::Success)
                    {
                        finish(true, "Account connected! You're signed in to SnoreSaber");
                    }
                    else
                    {
                        finish(false, SignInFailedMessage);
                    }
                });
            });
        });
    }

    void SignOut()
    {
        std::error_code removeError;
        std::filesystem::remove(SnoreSaber::Static::STEAM_KEY_PATH, removeError);
        if (removeError)
        {
            ERROR("Failed to delete quest credential: {:s}", removeError.message());
        }

        auto container = BSML::Helpers::GetDiContainer();
        auto gameSessionService = container ? container->TryResolve<GameSessionService*>() : nullptr;
        if (gameSessionService)
        {
            // same reset the soft-restart path uses: drops the session and player info
            gameSessionService->OnSoftRestart();
        }

        INFO("Signed out; quest credential removed");
    }
}
