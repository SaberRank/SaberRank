#pragma once

#include "Features/Players/Domain/LocalPlayerPanelState.hpp"
#include "Features/Players/Services/GameSessionService.hpp"
#include "Features/Players/Services/PlayerProfileService.hpp"

#include <beatsaber-hook/shared/utils/typedefs.h>
#include <custom-types/shared/macros.hpp>
#include <functional>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Players::Services, LocalPlayerPanelSession, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::GameSessionService*, _gameSessionService);
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Players::Services::PlayerProfileService*, _playerProfileService);
    DECLARE_CTOR(ctor,
                 SnoreSaber::Features::Players::Services::GameSessionService* gameSessionService,
                 SnoreSaber::Features::Players::Services::PlayerProfileService* playerProfileService);

  public:
    using StateChangedCallback = std::function<void(SnoreSaber::Data::LocalPlayerPanelState)>;

    SnoreSaber::Data::LocalPlayerPanelState Load();
    void Refresh(std::function<void(SnoreSaber::Data::LocalPlayerPanelState)> finished);
    void ApplyCurrentSettings();
    SnoreSaber::Data::LocalPlayerPanelState CurrentState() const;
    void SetStateChangedCallback(StateChangedCallback callback);

  private:
    void Publish(SnoreSaber::Data::LocalPlayerPanelState state);

    SnoreSaber::Data::LocalPlayerPanelState _currentState;
    StateChangedCallback _stateChanged;
};
