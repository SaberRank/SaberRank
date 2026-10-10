#pragma once

#include "Features/Replays/Format/ReplayFile.hpp"
#include "Features/Replays/ReplayPlaybackContext.hpp"
#include <GlobalNamespace/GameEnergyCounter.hpp>
#include <GlobalNamespace/GameEnergyUIPanel.hpp>
#include <GlobalNamespace/PlayerDataModel.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/UI/Image.hpp>
#include <UnityEngine/Vector3.hpp>
#include <Zenject/DiContainer.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <utility>
#include <vector>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::Playback, EnergyPlayer, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::GameEnergyCounter>, _gameEnergyCounter);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::GameEnergyUIPanel>, _gameEnergyUIPanel);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<GlobalNamespace::PlayerDataModel>, _playerDataModel);
    DECLARE_CTOR(ctor, SnoreSaber::ReplaySystem::ReplayPlaybackContext* replayContext, GlobalNamespace::GameEnergyCounter* gameEnergyCounter, GlobalNamespace::PlayerDataModel* playerDataModel, Zenject::DiContainer* container);
    DECLARE_INSTANCE_METHOD(void, TimeUpdate, float songTime);
    vector<Data::Private::EnergyEvent> _sortedEnergyEvents;
    std::vector<float> _energyDropTimes;
    void UpdateEnergy(float energy);
    void PrepareEnergyUIPanel();
    void UpdateEnergyUIPanel(float energy);
    void EnsureEnergyUIPanelReady();
    void UpdateEnergyIcons(float energy);
    void UpdateBatteryEnergyIcons();
    void CaptureInitialEnergyIconPositions();
    void CaptureInitialPosition(UnityEngine::UI::Image* image);
    void RestoreInitialEnergyIconPositions();
    void RestoreEnergyBarIconPositions(float energy);
    std::vector<std::pair<UnityW<UnityEngine::RectTransform>, UnityEngine::Vector3>> _initialEnergyIconPositions;
    bool _energyBarIconsResolved = false;
    UnityW<UnityEngine::Transform> _energyIconFull;
    UnityW<UnityEngine::Transform> _energyIconEmpty;
};
