#pragma once

#include "Features/Live/Compete/Domain/CompeteSongSelection.hpp"

#include <HMUI/ImageView.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <UnityEngine/MonoBehaviour.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <custom-types/shared/coroutine.hpp>

#include <memory>
#include <string>

namespace SnoreSaber::Features::Live::Compete::UI::Components
{
    // plain helper owned by the room view controller; every bound unity object is
    // also rooted by the view controller's il2cpp fields, so raw pointers are fine
    class CompeteSongPreview
    {
      public:
        std::string name;
        std::string detail;
        std::string difficulty;
        std::string duration = "--";
        std::string bpm = "--";
        std::string nps = "--";
        std::string notes = "--";
        std::string obstacles = "--";
        std::string bombs = "--";
        std::string njs = "--";
        std::string jumpDistance = "--";
        std::string stars = "--";
        bool isEmpty = true;

        bool IsActive() const
        {
            return !isEmpty;
        }

        void Bind(UnityEngine::MonoBehaviour* coroutineHost, HMUI::ImageView* coverImage, UnityEngine::UI::LayoutElement* textColumnLayout,
                  TMPro::TextMeshProUGUI* nameText, TMPro::TextMeshProUGUI* detailText, TMPro::TextMeshProUGUI* difficultyText,
                  UnityEngine::RectTransform* contentTransform);
        void SetSong(const std::shared_ptr<Domain::CompeteSongSelection>& song);
        void RefreshVisuals();

      private:
        UnityEngine::MonoBehaviour* _coroutineHost = nullptr;
        HMUI::ImageView* _coverImage = nullptr;
        UnityEngine::UI::LayoutElement* _textColumnLayout = nullptr;
        TMPro::TextMeshProUGUI* _nameText = nullptr;
        TMPro::TextMeshProUGUI* _detailText = nullptr;
        TMPro::TextMeshProUGUI* _difficultyText = nullptr;
        UnityEngine::RectTransform* _contentTransform = nullptr;
        std::shared_ptr<Domain::CompeteSongSelection> _song;
        int _coverRequestVersion = 0;

        custom_types::Helpers::Coroutine LoadCover(std::shared_ptr<Domain::CompeteSongSelection> song, int requestVersion);
        void UpdateLayout();
    };
}
