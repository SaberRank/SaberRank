#include <GlobalNamespace/BeatmapDifficulty.hpp>
#include <GlobalNamespace/EnvironmentInfoSO.hpp>
#include <HMUI/CurvedCanvasSettings.hpp>
#include "Features/Replays/UI/GameReplayUI.hpp"
#include <TMPro/TMP_FontAsset.hpp>
#include <UnityEngine/Canvas.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/RenderMode.hpp>
#include <UnityEngine/Resources.hpp>
#include "logging.hpp"
#include <sstream>

using namespace UnityEngine;
using namespace GlobalNamespace;

typedef std::basic_stringstream<char16_t> u16sstream;

DEFINE_TYPE(SnoreSaber::ReplaySystem::UI, GameReplayUI);

namespace SnoreSaber::ReplaySystem::UI
{
    void GameReplayUI::ctor()
    {
        INVOKE_CTOR();
    }

    void GameReplayUI::Construct(GameplayCoreSceneSetupData* gameplayCoreSceneSetupData, ReplayPlaybackContext* replayContext)
    {
        _gameplayCoreSceneSetupData = gameplayCoreSceneSetupData;
        _replayContext = replayContext;
    }

    void GameReplayUI::Start()
    {
        CreateReplayUI();
    }

    void GameReplayUI::CreateReplayUI()
    {
        if (!_replayContext)
        {
            ERROR("Cannot create replay UI without replay context");
            return;
        }

        auto beatmapLevel = _replayContext->GetBeatmapLevel();
        if (!beatmapLevel)
        {
            ERROR("Cannot create replay UI without beatmap level");
            return;
        }

        auto beatmapKey = _replayContext->GetBeatmapKey();
        StringW replayText;
        replayText += "REPLAY MODE - Watching ";
        replayText += _replayContext->GetPlayerName();
        replayText += " play ";
        replayText += beatmapLevel->songAuthorName;
        replayText += " - ";
        replayText += beatmapLevel->songName;
        replayText += " (";
        replayText += GetFriendlyDifficulty(beatmapKey.difficulty);
        replayText += ")";
        float timeScale = _replayContext->GetInitialTimeScale();
        if (timeScale != 1.0f) {
            stringstream ss;
            ss << std::setprecision(3) << " [" << timeScale * 100 << "%]";
            replayText += ss.str();
        }
        std::string friendlyMods = _replayContext->GetModifiers();
        if (friendlyMods != "") {
            replayText += " [";
            replayText += friendlyMods;
            replayText += "]";
        }
        auto _watermarkCanvas = GameObject::New_ctor("InGameReplayUI");
        auto targetEnvironmentInfo = _gameplayCoreSceneSetupData ? _gameplayCoreSceneSetupData->targetEnvironmentInfo.ptr() : nullptr;
        float watermarkHeight = targetEnvironmentInfo && (std::string)targetEnvironmentInfo->environmentName == "Interscope" ? 3.5f : 4.0f;
        _watermarkCanvas->transform->position = {0.0f, watermarkHeight, 12.0f};
        _watermarkCanvas->transform->localScale = {0.025f, 0.025f, 0.025f};

        auto _canvas = _watermarkCanvas->AddComponent<Canvas*>();
        _watermarkCanvas->AddComponent<HMUI::CurvedCanvasSettings*>();
        _canvas->renderMode = RenderMode::WorldSpace;
        _canvas->enabled = false;
        auto _text = CreateText(_canvas->transform.cast<RectTransform>(), replayText, {0, 10}, {100, 20}, 15.0f);
        _text->alignment = TMPro::TextAlignmentOptions::Center;
        auto rectTransform = _text->transform;
        rectTransform->SetParent(_canvas->transform, false);
        _canvas->enabled = true;
    }

    TMPro::TextMeshProUGUI* GameReplayUI::CreateText(UnityEngine::RectTransform* parent, StringW text, UnityEngine::Vector2 anchoredPosition, UnityEngine::Vector2 sizeDelta, float fontSize)
    {
        auto gameObject = GameObject::New_ctor("CustomUIText-SnoreSaber");
        gameObject->SetActive(false);
        auto textMeshProUGUI = gameObject->AddComponent<TMPro::TextMeshProUGUI*>();
        auto font = UnityEngine::Resources::FindObjectsOfTypeAll<TMPro::TMP_FontAsset*>()->First([](TMPro::TMP_FontAsset* t) { return t->name == "Teko-Medium SDF";});
        textMeshProUGUI->font = font;
        textMeshProUGUI->rectTransform->SetParent(parent, false);
        textMeshProUGUI->text = text;
        textMeshProUGUI->fontSize = fontSize;
        textMeshProUGUI->color = Color::get_white();
        textMeshProUGUI->rectTransform->anchorMin = {0.5f, 0.5f};
        textMeshProUGUI->rectTransform->anchorMax = {0.5f, 0.5f};
        textMeshProUGUI->rectTransform->sizeDelta = sizeDelta;
        textMeshProUGUI->rectTransform->anchoredPosition = anchoredPosition;
        gameObject->SetActive(true);
        return textMeshProUGUI;
    }

    std::string GameReplayUI::GetFriendlyDifficulty(GlobalNamespace::BeatmapDifficulty diff) {
        switch(diff) {
            case BeatmapDifficulty::Easy: return "Easy";
            case BeatmapDifficulty::Normal: return "Normal";
            case BeatmapDifficulty::Hard: return "Hard";
            case BeatmapDifficulty::Expert: return "Expert";
            case BeatmapDifficulty::ExpertPlus: return "Expert+";
            default: return "unknown";
        }
    }
} // namespace SnoreSaber::ReplaySystem::UI
