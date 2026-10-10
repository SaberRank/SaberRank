#include "Features/Leaderboards/UI/Components/CellClicker.hpp"

#include <GlobalNamespace/BasicUIAudioManager.hpp>
#include <UnityEngine/Resources.hpp>
#include <UnityEngine/Time.hpp>
#include <UnityEngine/Transform.hpp>
#include <UnityEngine/Vector3.hpp>

#include <custom-types/shared/coroutine.hpp>
#include <beatsaber-hook/shared/utils/typedefs-wrappers.hpp>
#include "Utils/OperatorOverloads.hpp"

using namespace HMUI;
using namespace UnityEngine;
using namespace UnityEngine::EventSystems;

DEFINE_TYPE(SnoreSaber::CustomTypes::Components, CellClicker);


namespace SnoreSaber::CustomTypes::Components {
    namespace
    {
        GlobalNamespace::BasicUIAudioManager* GetBasicUIAudioManager()
        {
            static SafePtrUnity<GlobalNamespace::BasicUIAudioManager> audioManager;
            if (!audioManager)
            {
                audioManager = Resources::FindObjectsOfTypeAll<GlobalNamespace::BasicUIAudioManager*>()->FirstOrDefault();
            }
            return audioManager.ptr();
        }

        void PlayButtonClickSound()
        {
            auto audioManager = GetBasicUIAudioManager();
            if (audioManager)
            {
                audioManager->HandleButtonClickEvent();
            }
        }
    }

    void CellClicker::Configure(int rowIndex, ImageView* rowSeparator, std::function<void(int)> clicked) {
        index = rowIndex;
        separator = rowSeparator;
        onClick = clicked;
        if (separator) {
            originalScale = separator->transform->localScale;
        }
    }

    void CellClicker::OnPointerClick(PointerEventData* data) {
        PlayButtonClickSound();

        if (onClick) {
            onClick(index);
        }
    }

    custom_types::Helpers::Coroutine LerpColors(ImageView* target, Color startColor, Color endColor, Color startColor0, Color endColor0, Color startColor1, Color endColor1, float duration) {
        float elapsedTime = 0.0f;
        while (elapsedTime < duration) {
            float t = elapsedTime / duration;
            target->color = Color::Lerp(startColor, endColor, t);
            target->color0 = Color::Lerp(startColor0, endColor0, t);
            target->color1 = Color::Lerp(startColor1, endColor1, t);
            elapsedTime += Time::get_deltaTime();
            co_yield nullptr;
        }
        target->color = endColor;
        target->color0 = endColor0;
        target->color1 = endColor1;
        co_return;
    }

    void CellClicker::OnPointerEnter(PointerEventData* eventData) {
        if (!separator) {
            return;
        }

        if (!isScaled) {
            separator->transform->localScale = originalScale * 1.8f;
            isScaled = true;
        }

        Color targetColor = Color::get_white();
        Color targetColor0 = Color::get_white();
        Color targetColor1 = Color(1, 1, 1, 0);

        float lerpDuration = 0.15f;

        StopAllCoroutines();
        StartCoroutine(custom_types::Helpers::CoroutineHelper::New(LerpColors(separator, separator->color, targetColor, separator->color0, targetColor0, separator->color1, targetColor1, lerpDuration)));
    }

    void CellClicker::OnPointerExit(PointerEventData* eventData) {
        if (!separator) {
            return;
        }

        if (isScaled) {
            separator->transform->localScale = originalScale;
            isScaled = false;
        }

        float lerpDuration = 0.05f;

        StopAllCoroutines();
        StartCoroutine(custom_types::Helpers::CoroutineHelper::New(LerpColors(separator, separator->color, origColour, separator->color0, origColour0, separator->color1, origColour1, lerpDuration)));
    }

    void CellClicker::OnDestroy() {
        StopAllCoroutines();
        onClick = nullptr;
        if (separator) {
            separator->transform->localScale = originalScale;
            separator->color = origColour;
            separator->color0 = origColour0;
            separator->color1 = origColour1;
        }
    }
}
