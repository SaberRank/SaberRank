#pragma once

#include "Core/Timing/SnoreSaberClock.hpp"
#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteRoom.hpp"
#include "Features/Live/Compete/Domain/CompeteSongSelection.hpp"
#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Features/Live/Protocol/Generated/Commands.hpp"

#include <GlobalNamespace/EnvironmentsListModel.hpp>
#include <GlobalNamespace/MenuTransitionsHelper.hpp>
#include <GlobalNamespace/PlayerDataModel.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayLauncher, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::PlayerDataModel*, _playerDataModel);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::MenuTransitionsHelper*, _menuTransitionsHelper);
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::EnvironmentsListModel*, _environmentsListModel);
    DECLARE_INSTANCE_FIELD_PRIVATE(CompeteGameplayState*, _gameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(Core::Timing::SnoreSaberClock*, _clock);
    DECLARE_CTOR(ctor,
                 GlobalNamespace::PlayerDataModel* playerDataModel,
                 GlobalNamespace::MenuTransitionsHelper* menuTransitionsHelper,
                 GlobalNamespace::EnvironmentsListModel* environmentsListModel,
                 CompeteGameplayState* gameplayState,
                 Core::Timing::SnoreSaberClock* clock);

public:
    // blocking; call off the main thread
    void Start(const Domain::CompeteRoom* room, int delayMs, const CancellationToken& cancellationToken);
    bool WaitForMapStartReady(const std::string& matchId, const std::string& mapHash, const CancellationToken& cancellationToken);
    int StartDelayMs(const ::SnoreSaber::Live::V1::ServerCommand* command);

private:
    void StartOnMainThread(const std::shared_ptr<Domain::CompeteSongSelection>& song, const std::string& tournamentId, const std::string& matchId);
};
