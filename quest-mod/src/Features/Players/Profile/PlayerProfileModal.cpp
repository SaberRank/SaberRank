#include "Features/Players/Profile/PlayerProfileModal.hpp"

#include "Core/Api/SnoreSaberUrls.hpp"
#include "Core/Presentation/PlayerPresentation.hpp"
#include "Core/Presentation/SnoreSaberUIMaterials.hpp"
#include "Features/Players/Profile/ProfileDetailData.hpp"
#include "Features/Players/Services/PlayerProfileService.hpp"
#include <HMUI/CurvedCanvasSettingsHelper.hpp>
#include <HMUI/ImageView.hpp>
#include "Sprites.hpp"
#include <UnityEngine/Application.hpp>
#include <UnityEngine/Networking/DownloadHandlerTexture.hpp>
#include <UnityEngine/Networking/UnityWebRequest.hpp>
#include <UnityEngine/Networking/UnityWebRequestTexture.hpp>
#include <UnityEngine/Rect.hpp>
#include <UnityEngine/RectOffset.hpp>
#include <UnityEngine/Sprite.hpp>
#include <UnityEngine/SpriteMeshType.hpp>
#include <UnityEngine/TextAnchor.hpp>
#include <UnityEngine/Texture2D.hpp>
#include <UnityEngine/UI/ContentSizeFitter.hpp>
#include <UnityEngine/UI/LayoutElement.hpp>
#include <bsml/shared/BSML/Components/Backgroundable.hpp>
#include <bsml/shared/BSML/SharedCoroutineStarter.hpp>
#include <bsml/shared/BSML-Lite.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include "Utils/UIUtils.hpp"
#include "Utils/SafePtr.hpp"
#include "Utils/WebUtils.hpp"
#include "logging.hpp"
#include "static.hpp"
#include "Utils/StrippedMethods.hpp"

#include <algorithm>
DEFINE_TYPE(SnoreSaber::UI::Other, PlayerProfileModal);

using namespace HMUI;
using namespace UnityEngine;
using namespace UnityEngine::Networking;
using namespace UnityEngine::UI;
using namespace BSML;
using namespace BSML::Lite;

#define SetPreferredSize(identifier, width, height)                                         \
    auto layout##identifier = identifier->gameObject->GetComponent<LayoutElement*>(); \
    if (!layout##identifier)                                                                \
        layout##identifier = identifier->gameObject->AddComponent<LayoutElement*>();  \
    layout##identifier->preferredWidth = width;                                          \
    layout##identifier->preferredHeight = height

#define BeginCoroutine(method) BSML::SharedCoroutineStarter::StartCoroutine(custom_types::Helpers::CoroutineHelper::New(method))

#define WIDTH 90.0f
#define HEIGHT 45.0f
namespace SnoreSaber::UI::Other
{
    custom_types::Helpers::Coroutine PlayerProfileModal::FetchPlayerData(std::string playerId)
    {
        if (playerId == "")
            co_return;

        auto profileService = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Players::Services::PlayerProfileService*>();
        if (!profileService)
        {
            co_return;
        }

        int requestId = profileRequestId;
        SafePtrUnity<PlayerProfileModal> self(this);
        profileService->GetPlayerInfoAsync(playerId, true, [self, requestId](std::optional<SnoreSaber::Data::Player> player) {
            if (!self || self->profileRequestId != requestId || !player.has_value())
                return;

            self->SetPlayerData(player.value());
        });

        co_return;
    }

    void PlayerProfileModal::SetPlayerData(SnoreSaber::Data::Player& player)
    {
        SetProfileData(ProfileDetailData::Create(player));
    }

    PlayerProfileModal* PlayerProfileModal::Create(UnityEngine::Transform* parent)
    {
        auto modal = CreateModal(parent, Vector2(WIDTH, HEIGHT), nullptr);
        auto ppmodal = modal->gameObject->AddComponent<PlayerProfileModal*>();
        ppmodal->modal = modal;
        ppmodal->Setup();
        return ppmodal;
    }

    void PlayerProfileModal::Hide()
    {
        profileRequestId++;
        modal->Hide(true, nullptr);
        stopProfileRoutine();
        stopBadgeRoutines();
    }

    void PlayerProfileModal::Show(std::string playerId)
    {
        this->playerId = playerId;
        profileRequestId++;
        modal->Show(true, true, nullptr);
        ClearBadges();
        ApplyCrown({});
        ResetProfileFont();
        BeginCoroutine(FetchPlayerData(playerId));
    }

    void PlayerProfileModal::Setup()
    {
        auto vertical = CreateVerticalLayoutGroup(transform);
        SetPreferredSize(vertical, WIDTH, HEIGHT);

        // header stuff
        auto headerHorizon = CreateHorizontalLayoutGroup(vertical->transform);
        SetPreferredSize(headerHorizon, 90, -1);

        auto bg = headerHorizon->gameObject->AddComponent<Backgroundable*>();
        bg->ApplyBackground("title-gradient");
        bg->ApplyAlpha(1.0f);

        auto bgImage = bg->gameObject->GetComponentInChildren<ImageView*>();
        bgImage->gradient = false;
        bgImage->color0 = Color(1, 1, 1, 1);
        bgImage->color1 = Color(1, 1, 1, 1);

        // placeholder color
        bgImage->color = Color(85 / 255.0f, 94 / 255.0f, 188 / 255.0f, 1);
        bgImage->_curvedCanvasSettingsHelper->Reset();

        auto prefixTexture = Texture2D::get_whiteTexture();
        auto prefixSprite = Sprite::Create(prefixTexture, Rect(0.0f, 0.0f, static_cast<float>(prefixTexture->width), static_cast<float>(prefixTexture->height)), Vector2(0.5f, 0.5f), 1024.0f, 1u,
                                           SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);
        prefixImage = CreateImage(headerHorizon->transform, prefixSprite, Vector2(0, 0), Vector2(0, 0));
        prefixImage->preserveAspect = true;
        SetPreferredSize(prefixImage, 5.5f, 5.5f);
        prefixHoverHint = AddHoverHint(prefixImage->gameObject, "");
        prefixHoverHint->enabled = false;
        prefixImage->gameObject->SetActive(false);

        headerText = CreateClickableText(headerHorizon->transform, u"Profile Placeholder", {0, 0}, {0, 0}, std::bind(&PlayerProfileModal::OpenPlayerUrl, this));
        SetPreferredSize(headerText, 82, -1);
        headerHorizon->childAlignment = TextAnchor::MiddleCenter;
        headerText->alignment = TMPro::TextAlignmentOptions::Center;
        // actual data stuff
        auto dataHorizon = CreateHorizontalLayoutGroup(vertical->transform);
        dataHorizon->padding = RectOffset::New_ctor(2, 2, 2, 2);
        SetPreferredSize(dataHorizon, 90.0f, 40.0f);
        dataHorizon->childForceExpandHeight = false;

        auto leftVertical = CreateVerticalLayoutGroup(dataHorizon->transform);
        auto seperatorVertical = CreateVerticalLayoutGroup(dataHorizon->transform);
        auto dataVertical = CreateVerticalLayoutGroup(dataHorizon->transform);
        leftVertical->childForceExpandHeight = false;

        SetPreferredSize(leftVertical, 35, -1);
        SetPreferredSize(seperatorVertical, 0.75f, 40.0f);
        SetPreferredSize(dataVertical, 40.0, 45.0f);

        // pfp setup
        auto pfpVertical = CreateVerticalLayoutGroup(leftVertical->transform);
        SetPreferredSize(pfpVertical, 35.0, -1);
        pfpVertical->childForceExpandHeight = true;
        auto contentSizeFitter = pfpVertical->gameObject->GetComponent<ContentSizeFitter*>();
        if (!contentSizeFitter)
            contentSizeFitter = pfpVertical->gameObject->AddComponent<ContentSizeFitter*>();
        contentSizeFitter->verticalFit = ContentSizeFitter::FitMode::Unconstrained;
        pfpVertical->padding = RectOffset::New_ctor(2, 2, 2, 2);
        auto oculusSprite = Base64ToSprite(oculus_base64);
        pfpImage = CreateImage(pfpVertical->transform, oculusSprite, Vector2(0, 0), Vector2(0, 0));
        pfpImage->preserveAspect = true;
        SetPreferredSize(pfpImage, -1, -1);

        badgeParent = CreateGridLayoutGroup(leftVertical->transform);
        badgeParent->cellSize = {9, 3.5};
        badgeParent->spacing = {2, 2};
        badgeParent->constraint = GridLayoutGroup::Constraint::Flexible;
        badgeParent->childAlignment = TextAnchor::MiddleCenter;

        SetPreferredSize(badgeParent, 42, 3.5f);

        // seperator setup
        auto texture = Texture2D::get_whiteTexture();
        auto seperatorSprite = Sprite::Create(texture, Rect(0.0f, 0.0f, (float)texture->width, (float)texture->height), Vector2(0.5f, 0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);

        auto image = CreateImage(seperatorVertical->transform, seperatorSprite, Vector2(0, 0), Vector2(0, 0));
        auto imageLayout = image->gameObject->AddComponent<LayoutElement*>();
        imageLayout->preferredWidth = 1.0f;

        // data setup
        CreateText(dataVertical->transform, "Global Player Ranking");
        globalRanking = CreateText(dataVertical->transform, "#Placeholder");
        CreateText(dataVertical->transform, "ZZ");
        performancePoints = CreateText(dataVertical->transform, "Placeholder ZZ");
        CreateText(dataVertical->transform, "Average Ranked Accuracy");
        averageRankedAccuracy = CreateText(dataVertical->transform, "Placeholder%");
        CreateText(dataVertical->transform, "Total Snores");
        totalScore = CreateText(dataVertical->transform, "Placeholder");

        badgeRoutines = ListW<UnityEngine::Coroutine*>::New();
        set_player(u"Placeholder");
        set_globalRanking(0);
        set_performancePoints(420.69f);
        set_averageRankedAccuracy(69.69f);
        set_totalScore(42042069);
    }

    void PlayerProfileModal::OpenPlayerUrl()
    {
        StrippedMethods::UnityEngine::Application::OpenURL(SnoreSaber::Core::Api::SnoreSaberUrls::Player(playerId));
    }

    void PlayerProfileModal::ClearBadges()
    {
        if (!badgeParent)
            return;
        auto transform = badgeParent->transform;
        int childCount = transform->childCount;
        for (int i = childCount - 1; i >= 0; i--)
        {
            auto child = transform->GetChild(i);
            auto go = child->gameObject;
            Object::DestroyImmediate(go);
        }
    }

    void PlayerProfileModal::AddBadge(SnoreSaber::Data::Badge& badge, int index)
    {
        AddBadge(ProfileBadgeData { badge.image, badge.description }, index);
    }

    void PlayerProfileModal::AddBadge(ProfileBadgeData const& badge, int index)
    {
        (void)index;
        auto texture = Texture2D::get_blackTexture();
        auto sprite = Sprite::Create(texture, Rect(0.0f, 0.0f, (float)texture->width, (float)texture->height), Vector2(0.5f, 0.5f), 1024.0f, 1u, SpriteMeshType::FullRect, Vector4(0.0f, 0.0f, 0.0f, 0.0f), false);

        auto image = CreateImage(badgeParent->transform, sprite, Vector2(0, 0), Vector2(0, 0));
        SetPreferredSize(image, 9, 3.5);
        image->preserveAspect = true;
        AddHoverHint(image->gameObject, badge.description);

        if (badge.image.ends_with(".gif"))
        {
            badgeRoutines->Add(BeginCoroutine(WebUtils::WaitForGifDownload(badge.image, image)));
        }
        else
        {
            badgeRoutines->Add(BeginCoroutine(WebUtils::WaitForImageDownload(badge.image, image)));
        }
    }

    void PlayerProfileModal::SetProfileData(ProfileDetailData const& player)
    {
        ApplyCrown(player.crown);
        ApplyProfileFont(player);
        set_player(player.displayName);
        globalRanking->text = "<i>" + player.rankText + "</i>";
        performancePoints->text = "<i>" + player.ppText + "</i>";
        averageRankedAccuracy->text = "<i>" + player.rankedAccuracyText + "</i>";
        totalScore->text = "<i>" + player.totalScoreText + "</i>";

        profileRoutine = BeginCoroutine(WebUtils::WaitForImageDownload(player.avatar, pfpImage));

        constexpr int maxBadges = 12;
        int badgeCount = std::min(static_cast<int>(player.badges.size()), maxBadges);
        for (int i = 0; i < badgeCount; ++i)
        {
            AddBadge(player.badges[i], i + 1);
        }

        playerId = player.player.id;
    }

    void PlayerProfileModal::ApplyCrown(ProfileCrownData const& crown)
    {
        if (!prefixImage || !prefixHoverHint)
        {
            return;
        }

        if (!crown.HasCrown())
        {
            prefixImage->gameObject->SetActive(false);
            prefixHoverHint->enabled = false;
            return;
        }

        auto sprite = SnoreSaber::Core::Presentation::PlayerPresentation::GetCrownSprite(crown.image);
        prefixImage->gameObject->SetActive(sprite);
        prefixHoverHint->enabled = sprite;
        if (!sprite)
        {
            return;
        }

        prefixImage->sprite = sprite;
        prefixHoverHint->text = crown.description;
    }

    void PlayerProfileModal::ApplyProfileFont(ProfileDetailData const& player)
    {
        if (!headerText)
        {
            return;
        }

        auto materials = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Core::Presentation::SnoreSaberUIMaterials*>();
        if (!materials)
        {
            return;
        }

        if (player.usesFurryFont)
        {
            if (auto material = materials->FurryFontMaterial())
            {
                headerText->set_fontMaterial(material);
                isFurryFontApplied = true;
            }
            return;
        }

        if (isFurryFontApplied)
        {
            ResetProfileFont();
        }
    }

    void PlayerProfileModal::ResetProfileFont()
    {
        if (!headerText)
        {
            return;
        }

        auto materials = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Core::Presentation::SnoreSaberUIMaterials*>();
        if (!materials)
        {
            return;
        }

        if (auto material = materials->DefaultFontMaterial())
        {
            headerText->set_fontMaterial(material);
        }
        isFurryFontApplied = false;
    }

    void PlayerProfileModal::set_player(std::u16string_view header)
    {
        set_header(std::u16string(header) + u"'s Profile");
    }

    void PlayerProfileModal::set_header(std::u16string_view header)
    {
        headerText->text = u"<i>" + std::u16string(header) + u"</i>";
    }

    void PlayerProfileModal::set_globalRanking(int globalRanking)
    {
        this->globalRanking->text = fmt::format("<i>#{:d}</i>", globalRanking);
    }

    void PlayerProfileModal::set_performancePoints(float performancePoints)
    {
        this->performancePoints->text = fmt::format("<i><color=#6772E5>{:.2f} ZZ</color></i>", performancePoints);
    }

    void PlayerProfileModal::set_averageRankedAccuracy(float averageRankedAccuracy)
    {
        this->averageRankedAccuracy->text = fmt::format("<i>{:.2f}%</i>", averageRankedAccuracy);
    }

    void PlayerProfileModal::set_totalScore(long totalScore)
    {
        this->totalScore->text = fmt::format("<i>{:L}</i>", totalScore);
    }

    void PlayerProfileModal::stopProfileRoutine()
    {
        if (profileRoutine)
            BSML::SharedCoroutineStarter::StopCoroutine(profileRoutine);
        profileRoutine = nullptr;
    }

    void PlayerProfileModal::stopBadgeRoutines()
    {
        if (badgeRoutines)
        {
            int length = badgeRoutines->Count;
            for (int i = 0; i < length; i++)
            {
                auto routine = badgeRoutines->get_Item(i);
                BSML::SharedCoroutineStarter::StopCoroutine(routine);
            }

            badgeRoutines->Clear();
        }
    }

} // namespace SnoreSaber::UI::Other
