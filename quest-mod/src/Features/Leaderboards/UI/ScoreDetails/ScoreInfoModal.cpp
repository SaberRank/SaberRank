#include "Features/Leaderboards/UI/ScoreDetails/ScoreInfoModal.hpp"

#include "Core/Presentation/PlayerPresentation.hpp"
#include "Features/Leaderboards/LeaderboardPresentationController.hpp"
#include "Features/Leaderboards/UI/ScoreDetails/ScoreDetailData.hpp"
#include "Sprites.hpp"
#include "assets.hpp"
#include <System/String.hpp>
#include "Features/Leaderboards/UI/SnoreSaberLeaderboardView.hpp"
#include <UnityEngine/RectOffset.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/SpriteMeshType.hpp>
#include <UnityEngine/SystemInfo.hpp>
#include <UnityEngine/Texture2D.hpp>
#include <UnityEngine/UI/HorizontalLayoutGroup.hpp>
#include <UnityEngine/UI/ContentSizeFitter.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <UnityEngine/UI/VerticalLayoutGroup.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include <bsml/shared/Helpers/utilities.hpp>

#include "Utils/AsyncUtils.hpp"
#include "Utils/SafePtr.hpp"

DEFINE_TYPE(SnoreSaber::UI::Other, ScoreInfoModal);

using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace GlobalNamespace;
using namespace TMPro;
using namespace System;
using namespace BSML;
using namespace BSML::Lite;

#define SetPreferredSize(identifier, width, height)                                         \
    auto layout##identifier = identifier->gameObject->GetComponent<LayoutElement*>(); \
    if (!layout##identifier)                                                                \
        layout##identifier = identifier->gameObject->AddComponent<LayoutElement*>();  \
    layout##identifier->preferredWidth = width;                                          \
    layout##identifier->preferredHeight = height

#define SetFitMode(identifier, horizontal, vertical)                                            \
    auto fitter##identifier = identifier->gameObject->GetComponent<ContentSizeFitter*>(); \
    fitter##identifier->verticalFit = vertical;                                              \
    fitter##identifier->horizontalFit = horizontal

#define CreateDefaultTextAndSetSize(identifier, size)                  \
    identifier = CreateText(textVertical->transform, "", FontStyles::Normal); \
    identifier->fontSize = size;

namespace SnoreSaber::UI::Other
{
    namespace
    {
        SnoreSaber::Features::Leaderboards::LeaderboardPresentationController* GetLeaderboardPresentationController()
        {
            return BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Leaderboards::LeaderboardPresentationController*>();
        }

        void SetLeaderboardPrompt(const std::string& text, int dismissTime)
        {
            auto presentationController = GetLeaderboardPresentationController();
            if (presentationController)
            {
                presentationController->SetPrompt(text, dismissTime);
            }
        }
    }

    void ScoreInfoModal::Hide()
    {
        replayRequestId++;
        modal->Hide(true, nullptr);
    }

    void ScoreInfoModal::Show(SnoreSaber::Data::ScoreMap& score)
    {
        replayRequestId++;
        this->leaderboardId = score.parent.leaderboard.id;

        auto scoreDetail = ScoreDetailData::Create(score);
        SetScoreInfo(scoreDetail);
        currentScore = scoreDetail.score.score;
        currentBeatmapLevel = score.parent.beatmapLevel;
        currentBeatmapKey = score.parent.beatmapKey;
        playerId = scoreDetail.playerId;

        replayEnabled = scoreDetail.hasReplay;
        replayFileName = score.replayFileName;
        modal->Show(true, true, nullptr);
        
        if(!SnoreSaberLeaderboardView::IsReplayWatchingAllowed())
            replayEnabled = false;

        SetReplayButtonState(replayEnabled);
    }

    ScoreInfoModal* ScoreInfoModal::Create(UnityEngine::Transform* parent, SnoreSaber::ReplaySystem::ReplayLoader* replayLoader)
    {
        auto modal = CreateModal(parent, Vector2(55, 50), nullptr);
        auto ppmodal = modal->gameObject->AddComponent<ScoreInfoModal*>();
        ppmodal->modal = modal;
        ppmodal->Setup(replayLoader);
        return ppmodal;
    }

    void ScoreInfoModal::Setup(SnoreSaber::ReplaySystem::ReplayLoader* replayLoader)
    {
        this->replayLoader = replayLoader;
        replayRequestId = 0;
        ContentSizeFitter::FitMode pref = ContentSizeFitter::FitMode::PreferredSize;

        auto mainVertical = CreateVerticalLayoutGroup(transform);
        SetPreferredSize(mainVertical, 55.0f, 50);
        SetFitMode(mainVertical, pref, pref);
        mainVertical->padding = RectOffset::New_ctor(3, 3, 3, 3);
        mainVertical->spacing = 0.8f;

        auto headerHorizontal = CreateHorizontalLayoutGroup(mainVertical->transform);
        SetPreferredSize(headerHorizontal, 50.0f, 5.0f);
        SetFitMode(headerHorizontal, pref, pref);

        auto nameVertical = CreateVerticalLayoutGroup(headerHorizontal->transform);
        // SetPreferredSize(nameVertical, 38.0f, 5.5f);
        SetFitMode(nameVertical, pref, pref);

        auto nameHorizontal = CreateHorizontalLayoutGroup(nameVertical->transform);
        SetPreferredSize(nameHorizontal, 38.0f, 5.5f);
        SetFitMode(nameHorizontal, pref, pref);
        nameHorizontal->childAlignment = TextAnchor::MiddleLeft;
        nameHorizontal->childForceExpandWidth = false;
        nameHorizontal->spacing = 1.0f;

        auto prefixTexture = Texture2D::get_whiteTexture();
        auto prefixSprite = Sprite::Create(prefixTexture, Rect(0.0f, 0.0f, static_cast<float>(prefixTexture->width), static_cast<float>(prefixTexture->height)), Vector2(0.5f, 0.5f), 1024.0f, 1u,
                                           SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
        prefixImage = CreateImage(nameHorizontal->transform, prefixSprite, Vector2(0, 0), Vector2(0, 0));
        prefixImage->preserveAspect = true;
        SetPreferredSize(prefixImage, 5.5f, 5.5f);
        prefixHoverHint = AddHoverHint(prefixImage->gameObject, "");
        prefixHoverHint->enabled = false;
        prefixImage->gameObject->SetActive(false);

        player = CreateText(nameHorizontal->transform, "", FontStyles::Normal);
        player->overflowMode = TextOverflowModes::Ellipsis;
        player->alignment = TextAlignmentOptions::Left;
        player->fontSize = 4.0f;

        auto buttonHorizontal = CreateHorizontalLayoutGroup(headerHorizontal->transform);
        SetPreferredSize(buttonHorizontal, 12.0f * 0.9f, 5.5f * 0.9f);
        buttonHorizontal->spacing = 0.2f;

        auto userSprite = Base64ToSprite(user_base64);
        auto userImage = CreateClickableImage(buttonHorizontal->transform, userSprite, std::bind(&ScoreInfoModal::ShowPlayerProfileModal, this), {0, 0}, {0, 0});
        userImage->preserveAspect = true;

        auto replaySprite = BSML::Utilities::LoadSpriteRaw(IncludedAssets::replay_png);
        replayImage = CreateClickableImage(buttonHorizontal->transform, replaySprite, std::bind(&ScoreInfoModal::PlayReplay, this), {0, 0}, {0, 0});
        replayImage->preserveAspect = true;

        auto separatorHorizontal = CreateHorizontalLayoutGroup(mainVertical->transform);
        auto separatorLayout = separatorHorizontal->gameObject->GetComponent<LayoutElement*>();
        if (!separatorLayout)
            separatorLayout = separatorHorizontal->gameObject->AddComponent<LayoutElement*>();

        separatorLayout->preferredHeight = 0.4f;

        auto texture = Texture2D::get_whiteTexture();
        auto whiteSprite = Sprite::Create(texture, Rect(0.0f, 0.0f, (float)texture->height, (float)texture->height), Vector2(0.5f, 0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
        auto separatorImage = CreateImage(separatorHorizontal->transform, whiteSprite, {0, 0}, {0, 0});
        separatorImage->rectTransform->sizeDelta = {48.0f, 0.4f};

        auto textVertical = CreateVerticalLayoutGroup(mainVertical->transform);
        SetPreferredSize(textVertical, 50.0f, 40.0f);
        textVertical->spacing = 0.1f;

        CreateDefaultTextAndSetSize(deviceHmd, 3.5f);
        CreateDefaultTextAndSetSize(deviceControllerLeft, 3.5f);
        CreateDefaultTextAndSetSize(deviceControllerRight, 3.5f);
        CreateDefaultTextAndSetSize(score, 3.5f);
        CreateDefaultTextAndSetSize(pp, 3.5f);
        CreateDefaultTextAndSetSize(combo, 3.5f);
        CreateDefaultTextAndSetSize(fullCombo, 3.5f);
        CreateDefaultTextAndSetSize(badCuts, 3.5f);
        CreateDefaultTextAndSetSize(missedNotes, 3.5f);
        CreateDefaultTextAndSetSize(modifiers, 3.5f);
        CreateDefaultTextAndSetSize(timeSet, 3.5f);

        ScoreDetailData placeholder;
        placeholder.playerNameText = u"placeholder's Snore";
        placeholder.deviceHmdText = "Unknown";
        placeholder.deviceControllerLeftText = "N/A";
        placeholder.deviceControllerRightText = "N/A";
        placeholder.scoreText = "0 (<color=#FFD42A>0.00%</color>)";
        placeholder.ppText = "<color=#6772E5>0.00 ZZ</color>";
        placeholder.maxComboText = "0";
        placeholder.fullComboText = "<color=#9EDBB1>Yes</color>";
        placeholder.badCutsText = "0";
        placeholder.missedNotesText = "0";
        placeholder.modifiersText = "";
        placeholder.timeSetText = "";
        SetScoreInfo(placeholder);
    }

    void ScoreInfoModal::ShowPlayerProfileModal()
    {
        if (!playerProfileModal)
        {
            return;
        }

        Hide();
        playerProfileModal->Show(playerId);
    }

    void ScoreInfoModal::SetReplayButtonState(bool enabled)
    {
        if (!replayImage)
        {
            return;
        }

        if (enabled)
        {
            // setting color doesn't seem to do anything, so I swapped the highlight colors around so that change only happens if enabled
            //replayImage->color = {1.0f, 1.0f, 1.0f, 0.8f};
            replayImage->highlightColor = {1.0f, 1.0f, 1.0f, 0.2f};
            return;
        }

        //replayImage->color = {1.0f, 1.0f, 1.0f, 0.2f};
        replayImage->highlightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    }

    void ScoreInfoModal::SetScoreInfo(ScoreDetailData const& score)
    {
        ApplyCrown(score);
        player->text = score.playerNameText;
        deviceHmd->text = fmt::format("<color=#6F6F6F>HMD:</color> {:s}", score.deviceHmdText);
        deviceControllerLeft->text = fmt::format("<color=#6F6F6F>Left Controller:</color> {:s}", score.deviceControllerLeftText);
        deviceControllerRight->text = fmt::format("<color=#6F6F6F>Right Controller:</color> {:s}", score.deviceControllerRightText);
        this->score->text = fmt::format("<color=#6F6F6F>Snore:</color> {:s}", score.scoreText);
        pp->text = fmt::format("<color=#6F6F6F>ZZ:</color> {:s}", score.ppText);
        combo->text = fmt::format("<color=#6F6F6F>Combo:</color> {:s}", score.maxComboText);
        fullCombo->text = fmt::format("<color=#6F6F6F>Full Combo:</color> {:s}", score.fullComboText);
        badCuts->text = fmt::format("<color=#6F6F6F>Bad Cuts:</color> {:s}", score.badCutsText);
        missedNotes->text = fmt::format("<color=#6F6F6F>Missed Notes:</color> {:s}", score.missedNotesText);
        modifiers->text = fmt::format("<color=#6F6F6F>Modifiers:</color> {:s}", score.modifiersText);
        timeSet->text = fmt::format("<color=#6F6F6F>Time Set:</color> {:s}", score.timeSetText);
    }

    void ScoreInfoModal::ApplyCrown(ScoreDetailData const& score)
    {
        if (!prefixImage || !prefixHoverHint)
        {
            return;
        }

        if (!score.HasCrown())
        {
            prefixImage->gameObject->SetActive(false);
            prefixHoverHint->enabled = false;
            return;
        }

        auto sprite = SnoreSaber::Core::Presentation::PlayerPresentation::GetCrownSprite(score.crownImage);
        prefixImage->gameObject->SetActive(sprite);
        prefixHoverHint->enabled = sprite;
        if (!sprite)
        {
            return;
        }

        prefixImage->sprite = sprite;
        prefixHoverHint->text = score.crownDescription;
    }

    void ScoreInfoModal::PlayReplay()
    {
        if (!replayEnabled)
        {
            return;
        }

        if (!replayLoader || !currentBeatmapLevel)
        {
            SetLeaderboardPrompt("Failed to load replay", 3);
            return;
        }

        SetReplayButtonState(false);
        replayEnabled = false;
        Hide();
        int requestId = ++replayRequestId;
        SafePtrUnity<ScoreInfoModal> self(this);
        SafePtr<SnoreSaber::ReplaySystem::ReplayLoader> replayLoaderSafe(replayLoader);
        SafePtr<BeatmapLevel> beatmapLevelSafe(currentBeatmapLevel);
        BeatmapKey beatmapKey = currentBeatmapKey;
        SetLeaderboardPrompt("Loading Replay...", -1);
        replayLoader->GetReplayData(currentBeatmapLevel, currentBeatmapKey, leaderboardId, replayFileName, currentScore, [self, replayLoaderSafe, beatmapLevelSafe, beatmapKey, requestId](SnoreSaber::ReplaySystem::ReplayLoadResult result) {
            SnoreSaber::Utils::Async::Main([self, replayLoaderSafe, beatmapLevelSafe, beatmapKey, requestId, result]() {
                if (!self || !replayLoaderSafe || self->replayRequestId != requestId)
                {
                    return;
                }

                if (result == SnoreSaber::ReplaySystem::ReplayLoadResult::Loaded)
                {
                    SetLeaderboardPrompt("Replay loaded!", 3);
                    replayLoaderSafe->StartReplay(beatmapLevelSafe.ptr(), beatmapKey);
                }
                else if (result == SnoreSaber::ReplaySystem::ReplayLoadResult::UnsupportedLegacy)
                {
                    SetLeaderboardPrompt("Unsupported replay version", 3);
                }
                else
                {
                    SetLeaderboardPrompt("Failed to load replay", 3);
                }
                self->SetReplayButtonState(true);
                self->replayEnabled = true;
            });
        });
    }
} // namespace SnoreSaber::UI::Other
