#pragma once

#include <HMUI/ImageView.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/EventSystems/IEventSystemHandler.hpp>
#include <UnityEngine/EventSystems/IPointerClickHandler.hpp>
#include <UnityEngine/EventSystems/IPointerEnterHandler.hpp>
#include <UnityEngine/EventSystems/IPointerExitHandler.hpp>
#include <UnityEngine/EventSystems/PointerEventData.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/Vector3.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::CustomTypes::Components,
        CellClicker,
        UnityEngine::MonoBehaviour,
        UnityEngine::EventSystems::IPointerClickHandler*,
        UnityEngine::EventSystems::IPointerEnterHandler*,
        UnityEngine::EventSystems::IPointerExitHandler*,
        UnityEngine::EventSystems::IEventSystemHandler*) {
    DECLARE_DEFAULT_CTOR();
    DECLARE_PRIVATE_METHOD(void, OnDestroy);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerClick, &UnityEngine::EventSystems::IPointerClickHandler::OnPointerClick, UnityEngine::EventSystems::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UnityEngine::EventSystems::IPointerEnterHandler::OnPointerEnter, UnityEngine::EventSystems::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UnityEngine::EventSystems::IPointerExitHandler::OnPointerExit, UnityEngine::EventSystems::PointerEventData* eventData);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, separator);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityEngine::Vector3, originalScale);
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(UnityEngine::Color, origColour, UnityEngine::Color(1, 1, 1, 1));
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(UnityEngine::Color, origColour0, UnityEngine::Color(1, 1, 1, 0.2509804f));
    DECLARE_INSTANCE_FIELD_PRIVATE_DEFAULT(UnityEngine::Color, origColour1, UnityEngine::Color(1, 1, 1, 0));

private:
    int index;
    std::function<void(int)> onClick;
    bool isScaled = false;

public:
    void Configure(int rowIndex, HMUI::ImageView* rowSeparator, std::function<void(int)> clicked);
};
