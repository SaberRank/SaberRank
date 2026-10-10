#include "Features/Replays/Playback/EnergyPlayer.hpp"
#include "Features/Replays/ReplayTime.hpp"
#include <GlobalNamespace/AudioTimeSyncController.hpp>
#include <GlobalNamespace/GameplayModifiers.hpp>
#include <GlobalNamespace/PlayerData.hpp>
#include <GlobalNamespace/PlayerSpecificSettings.hpp>
#include <System/Action_1.hpp>
#include <UnityEngine/Behaviour.hpp>
#include <UnityEngine/Component.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/Playables/PlayableDirector.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/Vector2.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/UI/Image.hpp>
#include <HMUI/ImageView.hpp>
#include "logging.hpp"
#include <algorithm>
#include <metacore/shared/internals.hpp>

using namespace UnityEngine;
using namespace SnoreSaber::Data::Private;
using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::ReplaySystem::Playback, EnergyPlayer);

namespace SnoreSaber::ReplaySystem::Playback
{
    namespace
    {
        constexpr float EnergyIconPositionX = 59.0f;
        constexpr auto LaserCloudName = "Laser";
        constexpr auto EnergyIconEmptyName = "EnergyIconEmpty";
        constexpr auto EnergyIconFullName = "EnergyIconFull";
        constexpr float EnergyIconTransparentAlpha = 0.251f;
    }

    void EnergyPlayer::ctor(ReplayPlaybackContext* replayContext, GlobalNamespace::GameEnergyCounter* gameEnergyCounter, GlobalNamespace::PlayerDataModel* playerDataModel, Zenject::DiContainer* container)
    {
        INVOKE_CTOR();
        _gameEnergyCounter = gameEnergyCounter;
        _playerDataModel = playerDataModel;
        _gameEnergyUIPanel = container->TryResolve<GlobalNamespace::GameEnergyUIPanel*>();
        _sortedEnergyEvents = replayContext->GetReplayFile()->energyKeyframes;
        std::stable_sort(_sortedEnergyEvents.begin(), _sortedEnergyEvents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.Time < rhs.Time;
        });

        float lastEnergy = 0.5f;
        for (const auto& energyEvent : _sortedEnergyEvents)
        {
            if (energyEvent.Energy < lastEnergy)
            {
                _energyDropTimes.push_back(energyEvent.Time);
            }
            lastEnergy = energyEvent.Energy;
        }
    }

    void EnergyPlayer::TimeUpdate(float newTime)
    {
        int nextIndex = ReplayTimeSearch::CountAtOrBefore(_sortedEnergyEvents, newTime, [](const auto& energyEvent) {
            return energyEvent.Time;
        });
        float energy = nextIndex > 0 ? _sortedEnergyEvents[nextIndex - 1].Energy : 0.5f;
        UpdateEnergy(energy);
    }

    void EnergyPlayer::UpdateEnergy(float energy)
    {
        bool isFailingEnergy = energy <= Mathf::getStaticF_Epsilon();
        bool noFail = _gameEnergyCounter->noFail;
        _gameEnergyCounter->noFail = false;
        _gameEnergyCounter->_didReach0Energy = isFailingEnergy;
        _gameEnergyCounter->_nextFrameEnergyChange = 0.0f;
        _gameEnergyCounter->energy = energy;
        _gameEnergyCounter->noFail = noFail;

        PrepareEnergyUIPanel();
        if (_gameEnergyCounter->gameEnergyDidChangeEvent)
        {
            _gameEnergyCounter->gameEnergyDidChangeEvent->Invoke(energy);
        }
        UpdateEnergyUIPanel(energy);
        
        // forgive me
        MetaCore::Internals::health = energy;

        MetaCore::Internals::wallsHit = ReplayTimeSearch::CountAtOrBefore(_energyDropTimes, MetaCore::Internals::audioTimeSyncController->songTime) > 0 ? 1 : 0;
        return;
    }

    void EnergyPlayer::PrepareEnergyUIPanel()
    {
        if (!_gameEnergyUIPanel || _playerDataModel->playerData->playerSpecificSettings->noTextsAndHuds)
        {
            return;
        }

        EnsureEnergyUIPanelReady();
        CaptureInitialEnergyIconPositions();
    }

    void EnergyPlayer::UpdateEnergyUIPanel(float energy)
    {
        if (!_gameEnergyUIPanel || _playerDataModel->playerData->playerSpecificSettings->noTextsAndHuds)
        {
            return;
        }

        auto director = _gameEnergyUIPanel->_playableDirector;
        if (director)
        {
            auto directorGameObject = director->get_gameObject();
            bool directorHasOwnObject = directorGameObject != _gameEnergyUIPanel->get_gameObject();
            if (directorHasOwnObject)
            {
                directorGameObject->SetActive(true);
            }

            director->set_enabled(true);
            director->Stop();
            director->set_time(0.0);
            director->Evaluate();

            if (directorHasOwnObject)
            {
                directorGameObject->SetActive(false);
            }
            else
            {
                director->set_enabled(false);
            }
        }

        UpdateEnergyIcons(energy);
        RestoreInitialEnergyIconPositions();
        RestoreEnergyBarIconPositions(energy);
    }

    void EnergyPlayer::EnsureEnergyUIPanelReady()
    {
        if (_gameEnergyCounter->energyType != GameplayModifiers::EnergyType::Battery || _gameEnergyUIPanel->_batteryLifeSegments)
        {
            return;
        }

        _gameEnergyUIPanel->Init();
    }

    void EnergyPlayer::UpdateEnergyIcons(float energy)
    {
        if (energy >= Mathf::getStaticF_Epsilon())
        {
            auto laserCloud = _gameEnergyUIPanel->get_transform()->Find(LaserCloudName);
            if (laserCloud)
            {
                laserCloud->get_gameObject()->SetActive(false);
            }
        }

        if (_gameEnergyCounter->energyType == GameplayModifiers::EnergyType::Battery)
        {
            UpdateBatteryEnergyIcons();
            return;
        }

        auto energyBar = _gameEnergyUIPanel->_energyBar;
        if (!energyBar)
        {
            return;
        }

        energyBar->get_gameObject()->SetActive(true);
        energyBar->set_enabled(true);
        energyBar->get_rectTransform()->anchorMax = Vector2(std::clamp(energy, 0.0f, 1.0f), 1.0f);
    }

    void EnergyPlayer::UpdateBatteryEnergyIcons()
    {
        auto energyBar = _gameEnergyUIPanel->_energyBar;
        if (energyBar)
        {
            energyBar->get_gameObject()->SetActive(false);
        }

        auto batteryLifeSegments = _gameEnergyUIPanel->_batteryLifeSegments;
        if (!batteryLifeSegments)
        {
            return;
        }

        int batteryEnergy = std::clamp(_gameEnergyCounter->batteryEnergy, 0, batteryLifeSegments->Count);
        for (int i = 0; i < batteryLifeSegments->Count; i++)
        {
            auto segment = batteryLifeSegments->get_Item(i);
            if (segment)
            {
                segment->set_enabled(i < batteryEnergy);
            }
        }

        _gameEnergyUIPanel->_activeBatteryLifeSegmentsCount = batteryEnergy;
    }

    void EnergyPlayer::CaptureInitialEnergyIconPositions()
    {
        CaptureInitialPosition(_gameEnergyUIPanel->_energyBar);

        auto batteryLifeSegments = _gameEnergyUIPanel->_batteryLifeSegments;
        if (!batteryLifeSegments)
        {
            return;
        }

        for (int i = 0; i < batteryLifeSegments->Count; i++)
        {
            CaptureInitialPosition(batteryLifeSegments->get_Item(i));
        }
    }

    void EnergyPlayer::CaptureInitialPosition(UnityEngine::UI::Image* image)
    {
        if (!image)
        {
            return;
        }

        UnityW<RectTransform> rectTransform = image->get_rectTransform();
        auto existing = std::find_if(_initialEnergyIconPositions.begin(), _initialEnergyIconPositions.end(), [rectTransform](const auto& initialPosition) {
            return initialPosition.first == rectTransform;
        });
        if (existing != _initialEnergyIconPositions.end())
        {
            return;
        }

        _initialEnergyIconPositions.emplace_back(rectTransform, rectTransform->anchoredPosition3D);
    }

    void EnergyPlayer::RestoreInitialEnergyIconPositions()
    {
        for (auto& [rectTransform, initialPosition] : _initialEnergyIconPositions)
        {
            if (rectTransform)
            {
                rectTransform->anchoredPosition3D = initialPosition;
            }
        }
    }

    void EnergyPlayer::RestoreEnergyBarIconPositions(float energy)
    {
        if (energy <= Mathf::getStaticF_Epsilon())
        {
            return;
        }

        if (!_energyBarIconsResolved)
        {
            auto transforms = _gameEnergyUIPanel->GetComponentsInChildren<Transform*>(true);
            for (auto transform : transforms)
            {
                if (transform->name == EnergyIconFullName)
                {
                    _energyIconFull = transform;
                }
                else if (transform->name == EnergyIconEmptyName)
                {
                    _energyIconEmpty = transform;
                }
            }

            _energyBarIconsResolved = true;
        }

        if (_energyIconFull)
        {
            _energyIconFull->localPosition = Vector3(EnergyIconPositionX, 0.0f, _energyIconFull->localPosition.z);
            auto image = _energyIconFull->GetComponent<HMUI::ImageView*>();
            image->color = Color(image->color.r, image->color.g, image->color.b, EnergyIconTransparentAlpha);
        }

        if (_energyIconEmpty)
        {
            _energyIconEmpty->localPosition = Vector3(-EnergyIconPositionX, 0.0f, _energyIconEmpty->localPosition.z);
            auto image = _energyIconEmpty->GetComponent<HMUI::ImageView*>();
            image->color = Color(image->color.r, image->color.g, image->color.b, EnergyIconTransparentAlpha);
        }
    }

} // namespace SnoreSaber::ReplaySystem::Playback
