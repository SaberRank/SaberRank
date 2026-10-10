#include "Features/Live/Compete/UI/Components/CompeteSongPreview.hpp"

#include "Utils/SafePtr.hpp"
#include "logging.hpp"

#include <GlobalNamespace/IPreviewMediaData.hpp>
#include <System/Threading/Tasks/Task_1.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/UI/LayoutRebuilder.hpp>

#include <fmt/core.h>

#include <algorithm>
#include <limits>

namespace SnoreSaber::Features::Live::Compete::UI::Components
{
    namespace
    {
        constexpr float MinSongTextWidth = 18.0f;
        constexpr float MaxSongTextWidth = 38.0f;

        float PreferredWidth(TMPro::TextMeshProUGUI* text, const std::string& value)
        {
            if (!text || value.empty())
            {
                return MinSongTextWidth;
            }

            text->ForceMeshUpdate(false, false);
            return text->GetPreferredValues(value, std::numeric_limits<float>::infinity(), 0.0f).x;
        }
    }

    void CompeteSongPreview::Bind(UnityEngine::MonoBehaviour* coroutineHost, HMUI::ImageView* coverImage, UnityEngine::UI::LayoutElement* textColumnLayout,
                                  TMPro::TextMeshProUGUI* nameText, TMPro::TextMeshProUGUI* detailText, TMPro::TextMeshProUGUI* difficultyText,
                                  UnityEngine::RectTransform* contentTransform)
    {
        _coroutineHost = coroutineHost;
        _coverImage = coverImage;
        _textColumnLayout = textColumnLayout;
        _nameText = nameText;
        _detailText = detailText;
        _difficultyText = difficultyText;
        _contentTransform = contentTransform;
    }

    void CompeteSongPreview::SetSong(const std::shared_ptr<Domain::CompeteSongSelection>& song)
    {
        _song = song;
        if (!song)
        {
            isEmpty = true;
            name = "";
            detail = "";
            difficulty = "";
            duration = "--";
            bpm = "--";
            nps = "--";
            notes = "--";
            obstacles = "--";
            bombs = "--";
            njs = "--";
            jumpDistance = "--";
            stars = "--";
            return;
        }

        isEmpty = false;
        name = song->name;
        detail = fmt::format("Mapped by {}", song->mapper);
        difficulty = fmt::format("{} / {}", song->difficulty, song->characteristic);
        duration = song->duration;
        bpm = song->bpm;
        nps = song->nps;
        notes = song->notes;
        obstacles = song->obstacles;
        bombs = song->bombs;
        njs = song->njs;
        jumpDistance = song->jumpDistance;
        stars = song->stars;
    }

    void CompeteSongPreview::RefreshVisuals()
    {
        UpdateLayout();
        int requestVersion = ++_coverRequestVersion;
        if (_coroutineHost)
        {
            _coroutineHost->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(LoadCover(_song, requestVersion)));
        }
    }

    custom_types::Helpers::Coroutine CompeteSongPreview::LoadCover(std::shared_ptr<Domain::CompeteSongSelection> song, int requestVersion)
    {
        if (!_coverImage)
        {
            co_return;
        }

        if (!song || !song->beatmapLevel)
        {
            _coverImage->sprite = nullptr;
            _coverImage->color = UnityEngine::Color::get_clear();
            co_return;
        }

        auto previewMediaData = song->beatmapLevel->previewMediaData;
        if (!previewMediaData)
        {
            ERROR("Failed to load compete song cover: level has no preview media data");
            co_return;
        }

        // pc awaits GetCoverSpriteAsync; quest polls the task from a coroutine, with
        // the task rooted since coroutine frames are invisible to the gc
        SafePtr<System::Threading::Tasks::Task_1<UnityW<UnityEngine::Sprite>>> task = previewMediaData->GetCoverSpriteAsync();
        while (!task->get_IsCompleted())
        {
            co_yield nullptr;
        }

        if (requestVersion != _coverRequestVersion || !_coverImage)
        {
            co_return;
        }

        if (task->get_IsFaulted() || task->get_IsCanceled())
        {
            ERROR("Failed to load compete song cover: cover task did not complete");
            co_return;
        }

        UnityEngine::Sprite* cover = task->get_Result().unsafePtr();
        _coverImage->sprite = cover;
        if (cover)
        {
            _coverImage->color = UnityEngine::Color::get_white();
        }
        else
        {
            _coverImage->color = UnityEngine::Color::get_clear();
        }
    }

    void CompeteSongPreview::UpdateLayout()
    {
        if (!_textColumnLayout)
        {
            return;
        }

        float textWidth = std::max({PreferredWidth(_nameText, name), PreferredWidth(_detailText, detail), PreferredWidth(_difficultyText, difficulty)});
        float width = std::clamp(textWidth, MinSongTextWidth, MaxSongTextWidth);
        _textColumnLayout->preferredWidth = width;
        _textColumnLayout->minWidth = width;

        if (_contentTransform)
        {
            UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(_contentTransform);
        }
    }
}
