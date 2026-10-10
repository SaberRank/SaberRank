#include "Features/Live/Compete/UI/ViewControllers/Room/Center/CompeteRoomViewController.hpp"

#include "assets.hpp"

#include <UnityEngine/RectTransform.hpp>
#include <bsml/shared/BSML-Lite.hpp>
#include <bsml/shared/BSML.hpp>

#include <fmt/core.h>

#include <algorithm>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Room::Center, CompeteRoomViewController);

namespace SnoreSaber::Features::Live::Compete::UI::ViewControllers::Room::Center
{
    namespace
    {
        std::string CountdownDetail(int remainingSeconds)
        {
            if (remainingSeconds <= 0)
            {
                return "Starting...";
            }

            return remainingSeconds == 1 ? "second" : "seconds";
        }

        void SetText(UnityW<TMPro::TextMeshProUGUI> text, const std::string& value)
        {
            if (text)
            {
                text->text = value;
            }
        }
    }

    void CompeteRoomViewController::ctor()
    {
        INVOKE_CTOR();
    }

    void CompeteRoomViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::CompeteRoomViewController_bsml, transform, this);
            BindSongPreview();
        }

        ApplyRoomTexts();
        ApplySongTexts();
        ApplyVisibility();
        _songPreview.RefreshVisuals();
    }

    bool CompeteRoomViewController::ReadyForPrompt()
    {
        return _parser && isInViewControllerHierarchy;
    }

    void CompeteRoomViewController::SetRoom(const Domain::CompeteRoom& room)
    {
        _roomTitle = room.DisplayName();
        _songStatus = room.songStatus;
        _readyText = room.localPlayerReady ? "Not Ready" : "Ready";
        _songPreview.SetSong(room.song);
        ApplyRoomTexts();
        ApplySongTexts();
        ApplyVisibility();
        _songPreview.RefreshVisuals();
    }

    void CompeteRoomViewController::ShowPrompt(const Domain::CompeteOrganizerPrompt& prompt)
    {
        _activePrompt = prompt;
        SetText(promptMessageText, prompt.message);
        if (promptPrimaryButton)
        {
            BSML::Lite::SetButtonText(promptPrimaryButton, prompt.primaryText);
        }

        if (promptSecondaryButton)
        {
            BSML::Lite::SetButtonText(promptSecondaryButton, prompt.secondaryText);
        }

        if (_parser && _parser->parserParams)
        {
            _parser->parserParams->EmitEvent("show-organiser-prompt");
        }
    }

    void CompeteRoomViewController::ClearPrompt()
    {
        _activePrompt.reset();
        if (_parser && _parser->parserParams)
        {
            _parser->parserParams->EmitEvent("hide-organiser-prompt");
        }
    }

    void CompeteRoomViewController::ShowMapStartCountdown(const Domain::CompeteMapStartCountdown& countdown)
    {
        SetText(countdownNumberText, fmt::format("{}", std::max(0, countdown.remainingSeconds)));
        SetText(countdownDetailText, CountdownDetail(countdown.remainingSeconds));
        if (!_parser || !_parser->parserParams)
        {
            return;
        }

        _parser->parserParams->EmitEvent("show-map-start-countdown");
    }

    void CompeteRoomViewController::HideMapStartCountdown()
    {
        if (!_parser || !_parser->parserParams)
        {
            return;
        }

        _parser->parserParams->EmitEvent("hide-map-start-countdown");
    }

    void CompeteRoomViewController::ToggleReadyClicked()
    {
        ReadyToggled.Invoke();
    }

    void CompeteRoomViewController::ShowPlayersClicked()
    {
        PlayersPanelSelected.Invoke();
    }

    void CompeteRoomViewController::ShowLeaderboardClicked()
    {
        LeaderboardPanelSelected.Invoke();
    }

    void CompeteRoomViewController::PromptPrimaryClicked()
    {
        AnswerPrompt(true);
    }

    void CompeteRoomViewController::PromptSecondaryClicked()
    {
        AnswerPrompt(false);
    }

    void CompeteRoomViewController::AnswerPrompt(bool accepted)
    {
        if (_parser && _parser->parserParams)
        {
            _parser->parserParams->EmitEvent("hide-organiser-prompt");
        }

        if (_activePrompt)
        {
            PromptAnswered.Invoke(*_activePrompt, accepted);
        }

        _activePrompt.reset();
    }

    void CompeteRoomViewController::BindSongPreview()
    {
        auto contentTransform = songContentObject ? songContentObject->GetComponent<UnityEngine::RectTransform*>() : nullptr;
        _songPreview.Bind(this, songCover.unsafePtr(), songTextColumn.unsafePtr(), songNameText.unsafePtr(), songDetailText.unsafePtr(),
                          songDifficultyText.unsafePtr(), contentTransform);
    }

    void CompeteRoomViewController::ApplyRoomTexts()
    {
        SetText(roomTitleText, _roomTitle);
        SetText(songStatusText, _songStatus);
        if (readyButton)
        {
            BSML::Lite::SetButtonText(readyButton, _readyText);
        }
    }

    void CompeteRoomViewController::ApplySongTexts()
    {
        SetText(songNameText, _songPreview.name);
        SetText(songDetailText, _songPreview.detail);
        SetText(songDifficultyText, _songPreview.difficulty);
        SetText(songDurationText, _songPreview.duration);
        SetText(songBpmText, _songPreview.bpm);
        SetText(songNpsText, _songPreview.nps);
        SetText(songNotesText, _songPreview.notes);
        SetText(songObstaclesText, _songPreview.obstacles);
        SetText(songBombsText, _songPreview.bombs);
        SetText(songNjsText, _songPreview.njs);
        SetText(songJumpDistanceText, _songPreview.jumpDistance);
        SetText(songStarsText, _songPreview.stars);
    }

    void CompeteRoomViewController::ApplyVisibility()
    {
        bool statusActive = !_songStatus.empty();
        if (songContentObject)
        {
            songContentObject->SetActive(_songPreview.IsActive() && !statusActive);
        }

        if (songStatusObject)
        {
            songStatusObject->SetActive(statusActive);
        }

        if (songEmptyObject)
        {
            songEmptyObject->SetActive(_songPreview.isEmpty && !statusActive);
        }
    }
}
