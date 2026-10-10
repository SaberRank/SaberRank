#pragma once

#include <HMUI/CurvedTextMeshPro.hpp>
#include "Features/Replays/UI/Components/AmeNode.hpp"
#include <UnityEngine/Camera.hpp>
#include <UnityEngine/Material.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/Vector2.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::ReplaySystem::UI::Components, AmeBar, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::RectTransform>, _rectTransform);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::RectTransform>, _fillBarTransform);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::CurvedTextMeshPro>, _endTimeText);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::CurvedTextMeshPro>, _currentTimeText);
    DECLARE_INSTANCE_FIELD(UnityW<UnityEngine::RectTransform>, _otherTransform);
    DECLARE_INSTANCE_METHOD(void, Setup, UnityEngine::RectTransform* fillBarTransform, UnityEngine::RectTransform* otherTransform);
    DECLARE_INSTANCE_METHOD(void, RegisterNode, SnoreSaber::ReplaySystem::UI::Components::AmeNode* node);
    DECLARE_INSTANCE_METHOD(void, UnregisterNode, SnoreSaber::ReplaySystem::UI::Components::AmeNode* node);
    DECLARE_INSTANCE_METHOD(float, GetNodePercent, SnoreSaber::ReplaySystem::UI::Components::AmeNode* node);
    DECLARE_INSTANCE_METHOD(void, AssignNodeToPercent, SnoreSaber::ReplaySystem::UI::Components::AmeNode* node, float percent);
    DECLARE_INSTANCE_METHOD(void, set_currentTime, float value);
    DECLARE_INSTANCE_METHOD(void, set_endTime, float value);
    DECLARE_INSTANCE_METHOD(void, set_barFill, float value);
    DECLARE_INSTANCE_METHOD(float, get_barFill);
    DECLARE_INSTANCE_METHOD(float, XForPercent, float percent);
    DECLARE_INSTANCE_METHOD(float, PercentForX, float x);
    DECLARE_DEFAULT_CTOR();
    void DragCallback(SnoreSaber::ReplaySystem::UI::Components::AmeNode* node, UnityEngine::Vector2 x, UnityEngine::Camera* camera);
    HMUI::CurvedTextMeshPro * CreateText();
private:
    int lastCurrentTimeSecond = -1;
};