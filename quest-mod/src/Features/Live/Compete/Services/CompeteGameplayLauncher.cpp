#include "Features/Live/Compete/Services/CompeteGameplayLauncher.hpp"

#include "Utils/AsyncUtils.hpp"
#include "logging.hpp"

#include <GlobalNamespace/BeatmapCharacteristicSO.hpp>
#include <GlobalNamespace/BeatmapKey.hpp>
#include <GlobalNamespace/BeatmapLevel.hpp>
#include <GlobalNamespace/ColorScheme.hpp>
#include <GlobalNamespace/ColorSchemesSettings.hpp>
#include <GlobalNamespace/GameplayModifiers.hpp>
#include <GlobalNamespace/LevelCompletionResults.hpp>
#include <GlobalNamespace/OverrideEnvironmentSettings.hpp>
#include <GlobalNamespace/PlayerData.hpp>
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include <GlobalNamespace/RecordingToolManager.hpp>
#include <GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp>
#include <System/Action_2.hpp>
#include <System/Nullable_1.hpp>
#include <custom-types/shared/delegate.hpp>

#include <chrono>
#include <future>
#include <limits>
#include <stdexcept>
#include <thread>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayLauncher);

namespace SnoreSaber::Features::Live::Compete::Services
{
    using namespace GlobalNamespace;

    namespace
    {
        constexpr int MapStartReadyPollMs = 25;
        constexpr int MapStartReadyTimeoutMs = 30000;

        GameplayModifiers* LiveGameplayModifiers()
        {
            return GameplayModifiers::New_ctor(
                GameplayModifiers::EnergyType::Bar,
                /* noFailOn0Energy */ true,
                /* instaFail */ false,
                /* failOnSaberClash */ false,
                GameplayModifiers::EnabledObstacleType::All,
                /* noBombs */ false,
                /* fastNotes */ false,
                /* strictAngles */ false,
                /* disappearingArrows */ false,
                GameplayModifiers::SongSpeed::Normal,
                /* noArrows */ false,
                /* ghostNotes */ false,
                /* proMode */ false,
                /* zenMode */ false,
                /* smallCubes */ false);
        }

        int ClampDelay(int64_t delayMs)
        {
            if (delayMs <= 0)
            {
                return 0;
            }

            return delayMs > std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : static_cast<int>(delayMs);
        }
    }

    void CompeteGameplayLauncher::ctor(
        PlayerDataModel* playerDataModel,
        MenuTransitionsHelper* menuTransitionsHelper,
        EnvironmentsListModel* environmentsListModel,
        CompeteGameplayState* gameplayState,
        Core::Timing::SnoreSaberClock* clock)
    {
        INVOKE_CTOR();
        _playerDataModel = playerDataModel;
        _menuTransitionsHelper = menuTransitionsHelper;
        _environmentsListModel = environmentsListModel;
        _gameplayState = gameplayState;
        _clock = clock;
    }

    void CompeteGameplayLauncher::Start(const Domain::CompeteRoom* room, int delayMs, const CancellationToken& cancellationToken)
    {
        if (!room)
        {
            throw std::invalid_argument("room");
        }

        std::shared_ptr<Domain::CompeteSongSelection> song = room->song;
        if (!song || !song->beatmapLevel)
        {
            throw std::runtime_error("Live room song is not installed");
        }

        if (delayMs > 0)
        {
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);
            while (std::chrono::steady_clock::now() < deadline)
            {
                cancellationToken.ThrowIfCancellationRequested();
                std::this_thread::sleep_for(std::chrono::milliseconds(MapStartReadyPollMs));
            }
        }

        // pc awaits the main thread and starts the level there; errors flow back to the caller
        std::string tournamentId = room->tournamentId;
        std::string matchId = room->id;
        auto done = std::make_shared<std::promise<void>>();
        Utils::Async::Main([this, song, tournamentId, matchId, done, cancellationToken] {
            try
            {
                cancellationToken.ThrowIfCancellationRequested();
                StartOnMainThread(song, tournamentId, matchId);
                done->set_value();
            }
            catch (...)
            {
                done->set_exception(std::current_exception());
            }
        });

        done->get_future().get();
    }

    void CompeteGameplayLauncher::StartOnMainThread(const std::shared_ptr<Domain::CompeteSongSelection>& song, const std::string& tournamentId, const std::string& matchId)
    {
        PlayerData* playerData = _playerDataModel->playerData;
        GameplayModifiers* modifiers = LiveGameplayModifiers();
        PlayerSpecificSettings* playerSettings = playerData->playerSpecificSettings;
        BeatmapLevel* beatmapLevel = song->beatmapLevel.ptr();
        BeatmapKey beatmapKey = *song->beatmapKey;

        _gameplayState->Begin(tournamentId, matchId, song->mapHash);
        try
        {
            auto levelFinishedDelegate = custom_types::MakeDelegate<System::Action_2<UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*>*>(
                classof(System::Action_2<UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*>*),
                std::function<void(UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*)>(
                    [state = _gameplayState](UnityW<StandardLevelScenesTransitionSetupDataSO>, LevelCompletionResults*) { state->End(); }));

            System::Nullable_1<RecordingToolManager_SetupData> recordingToolData;
            _menuTransitionsHelper->StartStandardLevel("Solo",
                                                       byref(beatmapKey),
                                                       beatmapLevel,
                                                       playerData->overrideEnvironmentSettings,
                                                       playerData->colorSchemesSettings->GetOverrideColorScheme(),
                                                       playerData->colorSchemesSettings->ShouldOverrideLightshowColors(),
                                                       beatmapLevel->GetColorScheme(beatmapKey.beatmapCharacteristic, beatmapKey.difficulty),
                                                       modifiers,
                                                       playerSettings,
                                                       nullptr,
                                                       _environmentsListModel,
                                                       "Menu",
                                                       false,
                                                       false,
                                                       nullptr,
                                                       nullptr,
                                                       levelFinishedDelegate,
                                                       nullptr,
                                                       recordingToolData);
        }
        catch (...)
        {
            _gameplayState->End();
            throw;
        }
    }

    bool CompeteGameplayLauncher::WaitForMapStartReady(const std::string& matchId, const std::string& mapHash, const CancellationToken& cancellationToken)
    {
        int waitedMs = 0;
        while (_gameplayState->IsCurrentMap(matchId, mapHash) && !_gameplayState->IsMapStartReady() && waitedMs < MapStartReadyTimeoutMs)
        {
            cancellationToken.ThrowIfCancellationRequested();
            std::this_thread::sleep_for(std::chrono::milliseconds(MapStartReadyPollMs));
            waitedMs += MapStartReadyPollMs;
        }

        if (!_gameplayState->IsCurrentMap(matchId, mapHash))
        {
            return false;
        }

        if (!_gameplayState->IsMapStartReady())
        {
            WARN("Ludus: Timed out waiting for FPS start gate; sending map start presence anyway.");
            _gameplayState->MarkMapStartReady();
        }

        return true;
    }

    int CompeteGameplayLauncher::StartDelayMs(const ::SnoreSaber::Live::V1::ServerCommand* command)
    {
        if (!command)
        {
            return 0;
        }

        int64_t now = _clock->UnixTimeMilliseconds();
        if (command->StartTimeUnixMs > now)
        {
            return ClampDelay(command->StartTimeUnixMs - now);
        }

        return ClampDelay(static_cast<int64_t>(command->CountdownMs));
    }
}
