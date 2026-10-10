#include "Features/Replays/Playback/ReplaySaberVisibility.hpp"

#include <GlobalNamespace/Saber.hpp>
#include <GlobalNamespace/SaberManager.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Transform.hpp>

namespace SnoreSaber::ReplaySystem::Playback::ReplaySaberVisibility
{
    namespace
    {
        bool ShouldShowSaber(GlobalNamespace::SaberManager* saberManager, GlobalNamespace::Saber* saber)
        {
            if (!saberManager || !saber)
                return false;

            auto initData = saberManager->_initData;
            if (!initData || !initData->oneSaberMode)
                return true;

            return saber->saberType.value__ == initData->oneSaberType.value__;
        }

        void EnsureSaberVisible(GlobalNamespace::SaberManager* saberManager, GlobalNamespace::Saber* saber)
        {
            if (!ShouldShowSaber(saberManager, saber))
                return;

            auto transform = saber->transform;
            while (transform)
            {
                auto gameObject = transform->gameObject;
                if (gameObject && !gameObject->activeSelf)
                    gameObject->SetActive(true);

                transform = transform->parent;
            }
        }
    }

    void EnsureVisible(GlobalNamespace::SaberManager* saberManager)
    {
        if (!saberManager)
            return;

        EnsureSaberVisible(saberManager, saberManager->leftSaber);
        EnsureSaberVisible(saberManager, saberManager->rightSaber);
    }
}
