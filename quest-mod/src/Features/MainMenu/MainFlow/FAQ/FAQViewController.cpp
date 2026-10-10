#include "Features/MainMenu/MainFlow/FAQ/FAQViewController.hpp"

#include "assets.hpp"
#include <UnityEngine/Application.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/RectOffset.hpp>
#include <UnityEngine/UI/HorizontalLayoutGroup.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <UnityEngine/UI/VerticalLayoutGroup.hpp>
#include <UnityEngine/UI/ContentSizeFitter.hpp>
#include "Utils/UIUtils.hpp"
#include <bsml/shared/BSML-Lite.hpp>
#include <bsml/shared/BSML/Components/Backgroundable.hpp>
#include <bsml/shared/Helpers/utilities.hpp>
#include <paper2_scotland2/shared/string_convert.hpp>
#include "Utils/StrippedMethods.hpp"
#include "Sprites.hpp"

#include <functional>

DEFINE_TYPE(SnoreSaber::UI::ViewControllers, FAQViewController);

using namespace SnoreSaber;
using namespace SnoreSaber::UI::ViewControllers;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace TMPro;
using namespace BSML;
using namespace BSML::Lite;

// clickable text
#define SNORESABER_LINK "https://snoresaber.com/"
// button
#define SNORESABER_DISCORD "https://discord.gg/snoresaber"
// button
#define SNORESABER_TWITTER "https://twitter.com/snoresaber"
// button
#define SNORESABER_PATREON "https://www.patreon.com/snoresaber"

// clickable text
#define BSMG_LINK "https://bsmg.wiki/"
// button
#define BSMG_DISCORD "https://discord.gg/beatsabermods"
// button
#define BSMG_TWITTER "https://twitter.com/beatsabermods"
// button
#define BSMG_PATREON "https://www.patreon.com/beatsabermods"

struct LinkBoxData
{
    std::string mainText;
    std::string mainURL;
    std::string discord;
    std::string twitter;
    std::string patreon;
    UnityEngine::Sprite* icon;
    UnityEngine::Color colora;
    UnityEngine::Color colorb;
    std::function<void()> iconClick;
};

HMUI::ImageView* CreateLinkBoxes(Transform* parent, LinkBoxData data)
{
    VerticalLayoutGroup* layoutGroup = CreateVerticalLayoutGroup(parent);
    layoutGroup->padding->bottom = 4;
    layoutGroup->padding->left = 4;
    layoutGroup->padding->right = 4;
    layoutGroup->padding->top = 4;

    ContentSizeFitter* layoutGroupFitter = layoutGroup->GetComponent<ContentSizeFitter*>();
    layoutGroupFitter->verticalFit = ContentSizeFitter::FitMode::PreferredSize;
    layoutGroupFitter->horizontalFit = ContentSizeFitter::FitMode::PreferredSize;

    Backgroundable* bg = layoutGroup->gameObject->AddComponent<Backgroundable*>();
    bg->ApplyBackground("title-gradient");
    bg->ApplyAlpha(1.0f);

    HMUI::ImageView* imageView = bg->gameObject->GetComponentInChildren<HMUI::ImageView*>();
    imageView->_skew = 0.0f;
    imageView->_gradient = true;
    imageView->_gradientDirection = 1;
    imageView->color = Color::get_white();
    imageView->_color0 = data.colora;
    imageView->_color1 = data.colorb;

    LayoutElement* layoutElement = layoutGroup->GetComponent<LayoutElement*>();
    layoutElement->preferredWidth = 40.0f;
    layoutElement->preferredHeight = 60.0f;

    ClickableText* text = CreateClickableText(layoutGroup->transform, Paper::StringConvert::from_utf8(data.mainText), [url = data.mainURL]() { StrippedMethods::UnityEngine::Application::OpenURL(url); });
    text->fontSize = 6.5f;
    text->alignment = TextAlignmentOptions::Center;

    CreateUIButton(layoutGroup->transform, "Discord", [url = data.discord]() { StrippedMethods::UnityEngine::Application::OpenURL(url); });
    CreateUIButton(layoutGroup->transform, "Twitter", [url = data.twitter]() { StrippedMethods::UnityEngine::Application::OpenURL(url); });
    CreateUIButton(layoutGroup->transform, "Patreon", [url = data.patreon]() { StrippedMethods::UnityEngine::Application::OpenURL(url); });

    VerticalLayoutGroup* imageLayout = CreateVerticalLayoutGroup(layoutGroup->transform);

    ContentSizeFitter* imageFitter = imageLayout->GetComponent<ContentSizeFitter*>();
    imageFitter->horizontalFit = ContentSizeFitter::FitMode::PreferredSize;
    imageFitter->verticalFit = ContentSizeFitter::FitMode::PreferredSize;

    VerticalLayoutGroup* imageParent = CreateVerticalLayoutGroup(imageLayout->transform);

    LayoutElement* imageElement = imageParent->GetComponent<LayoutElement*>();
    imageElement->preferredWidth = 20.0f;
    imageElement->preferredHeight = 20.0f;

    HMUI::ImageView* image = data.iconClick
        ? CreateClickableImage(imageElement->transform, data.icon, data.iconClick, {0.0f, 0.0f}, {0.0f, 0.0f})
        : CreateImage(imageElement->transform, data.icon, {0.0f, 0.0f}, {0.0f, 0.0f});
    image->preserveAspect = true;
    return image;
}

namespace SnoreSaber::UI::ViewControllers
{
    void FAQViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            VerticalLayoutGroup* vertical = CreateVerticalLayoutGroup(transform);
            ContentSizeFitter* sizeFitter = vertical->GetComponent<ContentSizeFitter*>();
            sizeFitter->horizontalFit = ContentSizeFitter::FitMode::PreferredSize;
            sizeFitter->verticalFit = ContentSizeFitter::FitMode::PreferredSize;
            vertical->spacing = 2.0f;

            auto headerHorizontal = CreateHorizontalLayoutGroup(vertical->transform);
            headerHorizontal->childAlignment = TextAnchor::MiddleCenter;
            auto headerText = CreateText(headerHorizontal->transform, "Links");
            headerText->alignment = TMPro::TextAlignmentOptions::Center;
            headerText->fontSize = 7.0f;
            auto headerBG = headerHorizontal->gameObject->AddComponent<Backgroundable*>();
            headerBG->ApplyBackground("round-rect-panel");
            headerBG->ApplyAlpha(0.5f);

            HorizontalLayoutGroup* horizontal = CreateHorizontalLayoutGroup(vertical->transform);

            ContentSizeFitter* horizFitter = horizontal->GetComponent<ContentSizeFitter*>();
            horizFitter->verticalFit = ContentSizeFitter::FitMode::PreferredSize;
            horizFitter->horizontalFit = ContentSizeFitter::FitMode::PreferredSize;

            LayoutElement* horizontalElement = horizontal->GetComponent<LayoutElement*>();
            horizontalElement->preferredWidth = 90.0f;

            LinkBoxData scoreSaber = {
                "SnoreSaber",
                SNORESABER_LINK,
                SNORESABER_DISCORD,
                SNORESABER_TWITTER,
                SNORESABER_PATREON,
                BSML::Utilities::LoadSpriteRaw(IncludedAssets::logo_large_png),
                Color(0.0f, 0.2f, 0.25f, 1.0f),
                Color(0.0f, 0.35f, 0.4f, 1.0f),
                std::bind(&FAQViewController::SnoreSaberImageClicked, this)};

            LinkBoxData bsmg = {
                "BSMG",
                BSMG_LINK,
                BSMG_DISCORD,
                BSMG_TWITTER,
                BSMG_PATREON,
                Base64ToSprite(bsmg_base64),
                Color(0.05f, 0.0f, 0.05f, 1.0f),
                Color(0.1f, 0.0f, 0.1f, 1.0f),
                std::bind(&FAQViewController::BsmgImageClicked, this)};

            scoreSaberImage = CreateLinkBoxes(horizontal->transform, scoreSaber);
            bsmgImage = CreateLinkBoxes(horizontal->transform, bsmg);
        }
    }

    void FAQViewController::SnoreSaberImageClicked()
    {
        if (!scoreSaberImage)
        {
            return;
        }

        scoreSaberCounter++;
        if (scoreSaberCounter == 5)
        {
            scoreSaberImage->sprite = BSML::Utilities::LoadSpriteRaw(IncludedAssets::logo_flushed_png);
        }
        if (scoreSaberCounter == 10)
        {
            scoreSaberImage->sprite = BSML::Utilities::LoadSpriteRaw(IncludedAssets::logo_large_png);
            scoreSaberCounter = 0;
        }
    }

    void FAQViewController::BsmgImageClicked()
    {
        if (!bsmgImage)
        {
            return;
        }

        bsmgCounter++;
        if (bsmgCounter == 5)
        {
            bsmgImage->sprite = BSML::Utilities::LoadSpriteRaw(IncludedAssets::cmb_png);
        }
        if (bsmgCounter == 10)
        {
            bsmgImage->sprite = BSML::Utilities::LoadSpriteRaw(IncludedAssets::cmb_blush_png);
        }
        if (bsmgCounter == 15)
        {
            bsmgImage->sprite = Base64ToSprite(bsmg_base64);
            bsmgCounter = 0;
        }
    }
} // namespace SnoreSaber::UI::ViewControllers
