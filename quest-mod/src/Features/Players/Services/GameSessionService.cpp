#include "Features/Players/Services/GameSessionService.hpp"

#include "Core/Api/SnoreSaberApiClient.hpp"
#include "Core/Presentation/PlayerPresentation.hpp"
#include "Core/Api/UploadTrust/UploadTrustBuildMetadata.hpp"
#include "Features/Players/Domain/GameAuthenticationRequest.hpp"
#include "Features/Players/Domain/GameAuthenticationResult.hpp"
#include "Features/Players/Domain/LocalPlayerInfo.hpp"
#include "Features/Players/Services/LocalPlayerPanelSession.hpp"
#include "Utils/AsyncUtils.hpp"
#include "Utils/Event.hpp"
#include "Utils/SafePtr.hpp"
#include "Utils/StringUtils.hpp"
#include "logging.hpp"
#include "static.hpp"

#include <System/Action.hpp>
#include <System/IO/Directory.hpp>
#include <beatsaber-hook/shared/config/rapidjson-utils.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <custom-types/shared/delegate.hpp>
#include <paper2_scotland2/shared/string_convert.hpp>

#include <mutex>
#include <utility>

using namespace StringUtils;
using namespace BSML;

DEFINE_TYPE(SnoreSaber::Features::Players::Services, GameSessionService);

namespace SnoreSaber::Features::Players::Services
{
    using SnoreSaber::Services::PlayerService::LoginStatus;

    namespace
    {
        struct PlayerInfo
        {
            std::string playerKey;
            std::string serverKey;
            std::optional<Core::Api::UploadTrust::UploadTrustSession> uploadTrust;
            LoginStatus loginStatus = LoginStatus::None;
            std::string loginStatusText;
            Data::Player localPlayerData;
            std::optional<Data::LocalPlayerInfo> localPlayerInfo;
            std::chrono::system_clock::time_point lastAuthenticatedAtUtc{};
        };

        PlayerInfo playerInfo;
        std::mutex authLock;
        SnoreSaber::Utils::Event<LoginStatus, const std::string&> loginStatusChanged;

        enum AuthState
        {
            NotStarted,
            Started,
            Finished
        } authState;
        int authGeneration = 0;
        std::vector<std::function<void(LoginStatus)>> finishedCallbacks;

        void ChangeLoginStatus(LoginStatus loginStatus, std::string statusText)
        {
            playerInfo.loginStatus = loginStatus;
            playerInfo.loginStatusText = statusText;
            // callers hold authLock; notify on the main thread like PC subscribers expect
            SnoreSaber::Utils::Async::Main([loginStatus, statusText = std::move(statusText)] {
                loginStatusChanged.Invoke(loginStatus, statusText);
            });
        }

        void NotifyAuthenticationFinished(LoginStatus loginStatus, int expectedGeneration = -1)
        {
            std::vector<std::function<void(LoginStatus)>> callbacks;
            {
                std::lock_guard lock(authLock);
                if (expectedGeneration != -1 && expectedGeneration != authGeneration)
                {
                    return;
                }

                authState = Finished;
                callbacks.swap(finishedCallbacks);
            }

            for (auto& callback : callbacks)
            {
                if (!callback)
                    continue;

                SnoreSaber::Utils::Async::Main([callback = std::move(callback), loginStatus] {
                    callback(loginStatus);
                });
            }
        }

        std::optional<Data::LocalPlayerInfo> ReadLocalPlayerInfo()
        {
            Core::Api::UploadTrust::UploadTrustBuildMetadata buildMetadata = Core::Api::UploadTrust::UploadTrustBuildMetadata::FromBuildConfig();
            bool useDevelopmentAuth = buildMetadata.HasDevelopmentAuth();

            Data::LocalPlayerInfo localPlayerInfo;
            localPlayerInfo.authType = useDevelopmentAuth ? 3 : 2;

            if (useDevelopmentAuth)
            {
                localPlayerInfo.nonce = buildMetadata.developmentAuthNonce;
                localPlayerInfo.playerId = buildMetadata.developmentPlayerId;
                localPlayerInfo.playerName = buildMetadata.developmentPlayerName;
            }
            else if (fileexists(SnoreSaber::Static::STEAM_KEY_PATH))
            {
                std::string rawAuthData = readfile(SnoreSaber::Static::STEAM_KEY_PATH);
                std::vector<std::string> splitAuthData = split(rawAuthData, ':');

                if (splitAuthData.size() > 1)
                {
                    localPlayerInfo.nonce = splitAuthData[0];
                    localPlayerInfo.playerId = splitAuthData[1];
                }
            }

            if (localPlayerInfo.playerId.empty() && fileexists(SnoreSaber::Static::STEAM_KEY_PATH))
            {
                std::string rawAuthData = readfile(SnoreSaber::Static::STEAM_KEY_PATH);
                std::vector<std::string> splitAuthData = split(rawAuthData, ':');
                if (splitAuthData.size() > 1)
                    localPlayerInfo.playerId = splitAuthData[1];
            }

            if (localPlayerInfo.nonce.empty() || localPlayerInfo.playerId.empty())
            {
                return std::nullopt;
            }

            if (!useDevelopmentAuth)
            {
                if (fileexists(SnoreSaber::Static::FRIENDS_PATH))
                {
                    localPlayerInfo.friends = readfile(SnoreSaber::Static::FRIENDS_PATH);
                }
                else
                {
                    writefile(SnoreSaber::Static::FRIENDS_PATH, "76561198283584459," + localPlayerInfo.playerId);
                    localPlayerInfo.friends = readfile(SnoreSaber::Static::FRIENDS_PATH);
                }
            }

            return localPlayerInfo;
        }

        Data::GameAuthenticationRequest CreateAuthenticateRequest(const Data::LocalPlayerInfo& localPlayerInfo)
        {
            Data::GameAuthenticationRequest request;
            request.authType = localPlayerInfo.authType;
            request.playerId = localPlayerInfo.playerId;
            request.nonce = localPlayerInfo.nonce;
            request.friendIds = localPlayerInfo.friends;
            request.playerName = localPlayerInfo.playerName;
            return request;
        }

        bool AuthenticateWithSnoreSaber(const Data::LocalPlayerInfo& localPlayerInfo, bool updateLocalPlayer, int expectedGeneration = -1)
        {
            Core::Api::SnoreSaberApiClient apiClient;
            Data::GameAuthenticationResult authResult = apiClient.AuthenticateGame(CreateAuthenticateRequest(localPlayerInfo));
            if (authResult.status != Data::GameSessionStatus::Success || !authResult.session.has_value() || !authResult.session->IsAuthenticated())
            {
                ERROR("SnoreSaber authentication failed: {:s}", authResult.message.c_str());
                return false;
            }

            std::lock_guard lock(authLock);
            if (expectedGeneration != -1 && expectedGeneration != authGeneration)
            {
                return false;
            }

            playerInfo.playerKey = authResult.session->sessionKey;
            playerInfo.serverKey = authResult.session->sessionId;
            playerInfo.uploadTrust = authResult.session->uploadTrust;
            playerInfo.lastAuthenticatedAtUtc = std::chrono::system_clock::now();

            if (updateLocalPlayer || playerInfo.localPlayerData.id.empty())
            {
                playerInfo.localPlayerData = Data::Player(localPlayerInfo.playerId);
                playerInfo.localPlayerData.name = Paper::StringConvert::from_utf8(localPlayerInfo.playerName);
            }

            if (updateLocalPlayer || !playerInfo.localPlayerInfo.has_value())
            {
                playerInfo.localPlayerInfo = localPlayerInfo;
            }

            return true;
        }

        void FinishWithError(std::string statusText, int expectedGeneration = -1)
        {
            {
                std::lock_guard lock(authLock);
                if (expectedGeneration != -1 && expectedGeneration != authGeneration)
                {
                    return;
                }

                ChangeLoginStatus(LoginStatus::Error, std::move(statusText));
            }
            NotifyAuthenticationFinished(LoginStatus::Error, expectedGeneration);
        }

    }

    void GameSessionService::ctor()
    {
        INVOKE_CTOR();
    }

    GameSessionService::LoginStatus GameSessionService::GetStatus() const
    {
        return playerInfo.loginStatus;
    }

    std::string GameSessionService::GetLocalPlayerId() const
    {
        return playerInfo.localPlayerData.id;
    }

    StringW GameSessionService::GetLocalPlayerName() const
    {
        return playerInfo.localPlayerData.name;
    }

    std::string GameSessionService::GetPlayerKey() const
    {
        return playerInfo.playerKey;
    }

    std::optional<SnoreSaber::Data::GameSession> GameSessionService::GetGameSession() const
    {
        if (playerInfo.loginStatus != LoginStatus::Success)
            return std::nullopt;

        Data::GameSession session;
        session.playerId = playerInfo.localPlayerData.id;
        if (playerInfo.localPlayerInfo.has_value() && !playerInfo.localPlayerInfo->playerName.empty())
            session.playerName = playerInfo.localPlayerInfo->playerName;
        else
            session.playerName = playerInfo.localPlayerData.plainName;
        session.sessionId = playerInfo.serverKey;
        session.sessionKey = playerInfo.playerKey;
        session.uploadTrust = playerInfo.uploadTrust;

        if (!session.IsAuthenticated())
            return std::nullopt;

        return session;
    }

    std::optional<SnoreSaber::Data::LocalPlayerInfo> GameSessionService::GetLocalPlayerInfo() const
    {
        return playerInfo.localPlayerInfo;
    }

    bool GameSessionService::HasAuthenticatedSession() const
    {
        return GetGameSession().has_value();
    }

    std::chrono::system_clock::time_point GameSessionService::LastAuthenticatedAtUtc() const
    {
        return playerInfo.lastAuthenticatedAtUtc;
    }

    uint64_t GameSessionService::AddLoginStatusChangedHandler(std::function<void(LoginStatus, const std::string&)> handler)
    {
        return loginStatusChanged.Add(std::move(handler));
    }

    void GameSessionService::RemoveLoginStatusChangedHandler(uint64_t token)
    {
        loginStatusChanged.Remove(token);
    }

    void GameSessionService::EnsureAuthenticated(bool forceRefresh, LoginCallback finished)
    {
        bool authenticated = false;
        int requestGeneration = 0;
        {
            std::lock_guard lock(authLock);
            if (!forceRefresh && playerInfo.loginStatus == LoginStatus::Success && GetGameSession().has_value())
            {
                authenticated = true;
            }
            else
            {
                finishedCallbacks.push_back(std::move(finished));
                if (authState == Started)
                    return;

                authState = Started;
                requestGeneration = authGeneration;
                ChangeLoginStatus(LoginStatus::InProgress, "Signing into SnoreSaber...");
            }
        }

        if (authenticated)
        {
            SnoreSaber::Utils::Async::Main([finished = std::move(finished)] {
                if (finished)
                {
                    finished(LoginStatus::Success);
                }
            });
            return;
        }

        std::optional<Data::LocalPlayerInfo> localPlayerInfo = ReadLocalPlayerInfo();
        if (!localPlayerInfo.has_value())
        {
            ERROR("Failed to read player authentication data");
            FinishWithError("Failed to read player authentication data", requestGeneration);
            return;
        }

        SnoreSaber::Utils::Async::Run([localPlayerInfo, requestGeneration] {
            try
            {
                if (!AuthenticateWithSnoreSaber(localPlayerInfo.value(), true, requestGeneration))
                {
                    FinishWithError("Failed to authenticate with SnoreSaber! Please restart your game", requestGeneration);
                    return;
                }

                {
                    std::lock_guard lock(authLock);
                    if (requestGeneration != authGeneration)
                    {
                        return;
                    }

                    ChangeLoginStatus(LoginStatus::Success, Core::Presentation::PlayerPresentation::GetLoginSuccessText(localPlayerInfo->playerId));
                }
                NotifyAuthenticationFinished(LoginStatus::Success, requestGeneration);
                SnoreSaber::Utils::Async::Main([]() {
                    auto gameSessionService = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Players::Services::GameSessionService*>();
                    if (gameSessionService)
                    {
                        gameSessionService->UpdatePlayerInfoThread();
                    }
                });
            }
            catch (const std::exception& exception)
            {
                ERROR("Authentication error: {:s}", exception.what());
                FinishWithError("Failed to authenticate with SnoreSaber! Please restart your game", requestGeneration);
            }
        });
    }

    bool GameSessionService::RefreshGameSession()
    {
        if (playerInfo.loginStatus != LoginStatus::Success)
            return false;

        std::optional<Data::LocalPlayerInfo> localPlayerInfo = ReadLocalPlayerInfo();
        if (!localPlayerInfo.has_value())
            return false;

        try
        {
            return AuthenticateWithSnoreSaber(localPlayerInfo.value(), false) && GetGameSession().has_value();
        }
        catch (const std::exception& exception)
        {
            ERROR("Failed to refresh SnoreSaber game session: {:s}", exception.what());
            return false;
        }
    }

    bool GameSessionService::RefreshUploadTrust()
    {
        return RefreshGameSession();
    }

    void GameSessionService::GetPlayerInfo(std::string playerId, bool full, std::function<void(std::optional<SnoreSaber::Data::Player>)> finished)
    {
        if (!finished)
            return;

        SnoreSaber::Utils::Async::RunThenMain([playerId = std::move(playerId), full] {
            try
            {
                Core::Api::SnoreSaberApiClient apiClient;
                return std::optional<SnoreSaber::Data::Player>(apiClient.GetPlayerProfile(playerId, full));
            }
            catch (const std::exception& exception)
            {
                ERROR("Failed to load SnoreSaber player: {:s}", exception.what());
                return std::optional<SnoreSaber::Data::Player>(std::nullopt);
            }
        },
        [finished = std::move(finished)](std::optional<SnoreSaber::Data::Player> player) {
            finished(player);
        });
    }

    void GameSessionService::UpdatePlayerInfoThread()
    {
        UpdatePlayerInfo();

        SafePtr<GameSessionService> self(this);
        SnoreSaber::Utils::Async::After(300.0f, [self] {
            if (self && self->GetStatus() == LoginStatus::Success)
            {
                self->UpdatePlayerInfoThread();
            }
        });
    }

    void GameSessionService::UpdatePlayerInfo()
    {
        auto panelSession = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Players::Services::LocalPlayerPanelSession*>();
        if (panelSession)
        {
            panelSession->Refresh([](Data::LocalPlayerPanelState state) {
                if (state.player.has_value())
                {
                    playerInfo.localPlayerData = state.player.value();
                    if (playerInfo.localPlayerInfo.has_value() && !state.player->plainName.empty())
                        playerInfo.localPlayerInfo->playerName = state.player->plainName;
                }
            });
            return;
        }

        GetPlayerInfo(playerInfo.localPlayerData.id, true, [](std::optional<Data::Player> playerData) {
            if (playerData.has_value())
            {
                playerInfo.localPlayerData = playerData.value();
                if (playerInfo.localPlayerInfo.has_value() && !playerData->plainName.empty())
                    playerInfo.localPlayerInfo->playerName = playerData->plainName;
            }
        });
    }

    void GameSessionService::OnSoftRestart()
    {
        std::lock_guard lock(authLock);
        authGeneration++;
        playerInfo = PlayerInfo();
        authState = NotStarted;
        finishedCallbacks.clear();
    }
}
