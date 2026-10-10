#include "Features/Replays/UI/Components/AmeClicker.hpp"
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/Rect.hpp>
#include <UnityEngine/RectTransformUtility.hpp>
#include "logging.hpp"

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::ReplaySystem::UI::Components, AmeClicker);

namespace SnoreSaber::ReplaySystem::UI::Components
{
    void AmeClicker::Setup(std::function<void(float)> callback)
    {
        _rect = transform->GetComponent<RectTransform*>();
        _clickCallback = callback;
    }
    void AmeClicker::OnPointerClick(UnityEngine::EventSystems::PointerEventData* eventData)
    {
        Vector2 computedVector;
        RectTransformUtility::ScreenPointToLocalPointInRectangle(_rect, eventData->position, eventData->pressEventCamera, byref(computedVector));
        float maxX = _rect->rect.width / 2.0f;
        _clickCallback(Mathf::InverseLerp(-maxX, maxX, computedVector.x));
    }
} // namespace SnoreSaber::ReplaySystem::UI::Components