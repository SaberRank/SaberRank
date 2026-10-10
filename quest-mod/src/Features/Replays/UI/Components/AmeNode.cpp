#include "Features/Replays/UI/Components/AmeNode.hpp"
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/RectTransformUtility.hpp>
#include <UnityEngine/Vector2.hpp>

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::ReplaySystem::UI::Components, AmeNode);

namespace SnoreSaber::ReplaySystem::UI::Components
{
    bool AmeNode::get_isBeingDragged()
    {
        return _handle->dragged;
    }
    void AmeNode::Init(SnoreSaber::ReplaySystem::UI::Components::AmeHandle* handle)
    {
        _handle = handle;
    }
    void AmeNode::AddCallback(std::function<void(AmeNode*, Vector2, Camera*)> callback)
    {
        _callback = callback;
        _handle->AddCallback([=, this](SnoreSaber::ReplaySystem::UI::Components::AmeHandle* handle, UnityEngine::Vector2 x, UnityEngine::Camera* camera) {
            Callback(handle, x, camera);
        });
    }
    void AmeNode::Callback(SnoreSaber::ReplaySystem::UI::Components::AmeHandle* handle, UnityEngine::Vector2 x, UnityEngine::Camera* camera)
    {
        if (!moveable || !_callback)
        {
            return;
        }

        _callback(this, x, camera);
    }
    void AmeNode::SendUpdatePositionCall(float percentOnBar)
    {
        if (PositionDidChange)
        {
            PositionDidChange(percentOnBar);
        }
    }
} // namespace SnoreSaber::ReplaySystem::UI::Components
