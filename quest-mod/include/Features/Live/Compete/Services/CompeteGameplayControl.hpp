#pragma once

#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"

#include <GlobalNamespace/PauseController.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>

#include <string>

// pc holds SiraUtil ISongControl; quest quits through PauseController directly
DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::Services, CompeteGameplayControl, System::Object) {
    DECLARE_INSTANCE_FIELD_PRIVATE(CompeteGameplayState*, _gameplayState);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PauseController>, _pauseController);
    DECLARE_CTOR(ctor, CompeteGameplayState* gameplayState);

public:
    void Register(GlobalNamespace::PauseController* pauseController);
    void Unregister(GlobalNamespace::PauseController* pauseController);
    bool TryStopMap(const std::string& matchId);
};

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::Features::Live::Compete::Services,
        CompeteGameplayControlBinder,
        System::Object,
        Zenject::IInitializable*,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(CompeteGameplayControl*, _gameplayControl);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PauseController>, _pauseController);
    DECLARE_CTOR(ctor, CompeteGameplayControl* gameplayControl, GlobalNamespace::PauseController* pauseController);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);
};
