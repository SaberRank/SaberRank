#include "Features/Replays/UI/MainImberPanelView.hpp"
#include <HMUI/ViewController.hpp>
#include <UnityEngine/Mathf.hpp>
#include <UnityEngine/RectTransformUtility.hpp>
#include <UnityEngine/Vector2.hpp>
#include "assets.hpp"
#include <bsml/shared/BSML.hpp>
#include "logging.hpp"
#include <sstream>
#include <iomanip>

using namespace UnityEngine;
using namespace GlobalNamespace;

DEFINE_TYPE(SnoreSaber::ReplaySystem::UI, MainImberPanelView);

namespace SnoreSaber::ReplaySystem::UI
{

    // Props

    Transform* MainImberPanelView::get_Transform()
    {
        return _floatingScreen->transform;
    }

    bool MainImberPanelView::get_visibility()
    {
        return _visible;
    }

    void MainImberPanelView::set_visibility(bool value)
    {
        _visible = value;
        if (_floatingScreen)
        {
            _floatingScreen->SetRootViewController(value ? this : nullptr, value ? HMUI::ViewController::AnimationType::In : HMUI::ViewController::AnimationType::Out);
        }
    }

    void MainImberPanelView::set_fps(int value)
    {
        if (!fpsText)
        {
            return;
        }

        fpsText->text = std::to_string(value);
        if (value > 0.85f * _targetFPS)
        {
            fpsText->color = _goodColor;
        }
        else if (value > 0.6f * _targetFPS)
        {
            fpsText->color = _ehColor;
        }
        else
        {
            fpsText->color = _noColor;
        }
    }

    void MainImberPanelView::set_leftSaberSpeed(float value)
    {
        if (!leftSpeedText)
        {
            return;
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << value << " m/s";
        leftSpeedText->text = ss.str();
        leftSpeedText->color = value >= 2.0f ? _goodColor : _noColor;
    }

    void MainImberPanelView::set_rightSaberSpeed(float value)
    {
        if (!rightSpeedText)
        {
            return;
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << value << " m/s";
        rightSpeedText->text = ss.str();
        rightSpeedText->color = value >= 2.0f ? _goodColor : _noColor;
    }

    void MainImberPanelView::set_didParse(bool value)
    {
        _didParse = value;
    }

    bool MainImberPanelView::get_didParse()
    {
        return _didParse;
    }

    void MainImberPanelView::set_timeSync(float value)
    {
        _timeSync = Mathf::Approximately(_initialTime, value) ? _initialTime : value;
        if (DidTimeSyncChange)
        {
            DidTimeSyncChange(_timeSync);
        }
    }

    float MainImberPanelView::get_timeSync()
    {
        return _timeSync;
    }

    void MainImberPanelView::set_loopText(StringW value)
    {
        _loopText = value;
    }

    StringW MainImberPanelView::get_loopText()
    {
        return _loopText;
    }

    void MainImberPanelView::set_playPauseText(StringW value)
    {
        _playPauseText = value;
    }

    StringW MainImberPanelView::get_playPauseText()
    {
        return _playPauseText;
    }

    void MainImberPanelView::set_location(StringW value)
    {
        _location = value;
        if (DidPositionPreviewChange)
        {
            DidPositionPreviewChange(_location);
        }
    }

    StringW MainImberPanelView::get_location()
    {
        return _location;
    }

    // Functions

    void MainImberPanelView::ctor()
    {
        INVOKE_CTOR();
        _playPauseText = "PAUSE";
        _loopText = "LOOP";
        _location = "";
    }


    void MainImberPanelView::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation) {
            BSML::parse_and_construct(IncludedAssets::imber_panel_bsml, transform, this);
        }
        didSelectDelegate = { &MainImberPanelView::DidSelect, this };
        tabSelector->textSegmentedControl->___didSelectCellEvent += didSelectDelegate;
        set_didParse(true);
        if (firstActivation)
        {
            Vector3 localScale = tabSelector->transform->localScale;
            tabSelector->transform->localScale = {localScale.x * 0.9f, localScale.y * 0.9f, localScale.z * 0.9f};
        }
    }

    void MainImberPanelView::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling)
    {
        if(tabSelector && UnityW<HMUI::SegmentedControl>::isAlive(tabSelector->textSegmentedControl))
            tabSelector->textSegmentedControl->___didSelectCellEvent -= didSelectDelegate;
    }

    void MainImberPanelView::Construct()
    {
        _floatingScreen = BSML::FloatingScreen::CreateFloatingScreen({60.0f, 45.0f}, false, defaultPosition.position, defaultPosition.rotation, 0.0f, false);

        auto localScale = _floatingScreen->transform->localScale;
        _floatingScreen->GetComponent<Canvas*>()->sortingOrder = 31;
        _floatingScreen->transform->localScale = {localScale.x / 2.0f, localScale.y / 2.0f, localScale.z / 2.0f};
        _floatingScreen->name = "Imber Replay Panel (Screen)";
        name = "Imber Replay Panel (View)";
    }

    void MainImberPanelView::Setup(float initialSongTime, int targetFramerate, std::string defaultLocation, std::vector<std::string> _locations)
    {
        _initialTime = initialSongTime;
        _targetFPS = targetFramerate;
        _timeSync = initialSongTime;
        _location = defaultLocation;
        for (auto &loc : _locations) {
            this->locations->Add(StringW(loc));
        }
    }

    void MainImberPanelView::DidSelect(UnityW<HMUI::SegmentedControl> segmentedControl, int selected)
    {
        int positionTabIndex = 2;
        if (_lastTab == 2 || selected == 2)
        {
            if (DidPositionTabVisibilityChange)
            {
                DidPositionTabVisibilityChange(selected == positionTabIndex);
            }
        }
        _lastTab = selected;
    }

    void MainImberPanelView::SwitchHand(XR::XRNode xrNode)
    {
        if (HandDidSwitchEvent)
        {
            HandDidSwitchEvent(xrNode);
        }
    }

    void MainImberPanelView::PausePlay()
    {
        if (DidClickPausePlay)
        {
            DidClickPausePlay();
        }
    }

    void MainImberPanelView::Restart()
    {
        if (DidClickRestart)
        {
            DidClickRestart();
        }
    }

    void MainImberPanelView::Loop()
    {
        if (DidClickLoop)
        {
            DidClickLoop();
        }
    }

    void MainImberPanelView::SwitchHandLeft()
    {
        SwitchHand(XR::XRNode::LeftHand);
    }

    void MainImberPanelView::SwitchHandRight()
    {
        SwitchHand(XR::XRNode::RightHand);
    }

    void MainImberPanelView::RequestDismiss()
    {
        if (DidChangeVisiblity)
        {
            DidChangeVisiblity(false);
        }
    }

    StringW MainImberPanelView::FormatTimePercent(float value)
    {
        return std::to_string(value);
    }

    void MainImberPanelView::Jump()
    {
        if (DidPositionJump)
        {
            DidPositionJump();
        }
    }

} // namespace SnoreSaber::ReplaySystem::UI
