#pragma once

#include "Features/Live/Compete/Services/CompeteGameplayState.hpp"
#include "Utils/DelegateUtils.hpp"

#include <GlobalNamespace/PauseController.hpp>
#include <System/Action_1.hpp>
#include <System/IDisposable.hpp>
#include <Zenject/IInitializable.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::Features::Live::Compete::Services,
        CompetePauseGuard,
        System::Object,
        Zenject::IInitializable*,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PauseController>, _pauseController);
    DECLARE_INSTANCE_FIELD_PRIVATE(CompeteGameplayState*, _gameplayState);
    DECLARE_CTOR(ctor, GlobalNamespace::PauseController* pauseController, CompeteGameplayState* gameplayState);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);

private:
    DelegateUtils::DelegateW<System::Action_1<System::Action_1<bool>*>> _canPauseDelegate;

    void CanPause(System::Action_1<bool>* canPause);
};
