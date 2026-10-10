#pragma once

#include <GlobalNamespace/PauseMenuManager.hpp>
#include <GlobalNamespace/VRController.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::UI, VRControllerAccessor, Il2CppObject) {
    DECLARE_INSTANCE_FIELD(UnityW<GlobalNamespace::VRController>, _leftController);
    DECLARE_INSTANCE_FIELD(UnityW<GlobalNamespace::VRController>, _rightController);
    DECLARE_CTOR(ctor, GlobalNamespace::PauseMenuManager*);
};