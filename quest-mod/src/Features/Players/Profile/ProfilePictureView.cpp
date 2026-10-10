#include "Features/Players/Profile/ProfilePictureView.hpp"

#include "Core/Presentation/RemoteImageService.hpp"
#include "Core/Presentation/SnoreSaberUIMaterials.hpp"
#include "Features/Leaderboards/Services/LeaderboardTweeningService.hpp"

#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Material.hpp>
#include <bsml/shared/Helpers/getters.hpp>
#include "Utils/OperatorOverloads.hpp"

using namespace System::Threading;
using namespace UnityEngine;
using namespace std;

namespace SnoreSaber::UI::Other {

    bool initializedGlobals = false;
    SafePtrUnity<Sprite> nullSprite;
    SafePtrUnity<Material> mat_UINoGlowRoundEdge;
    SnoreSaber::Core::Presentation::SnoreSaberUIMaterials* uiMaterials = nullptr;
    SnoreSaber::Core::Presentation::RemoteImageService* remoteImageService = nullptr;
    SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService* leaderboardTweeningService = nullptr;

    void FadeInAvatar(HMUI::ImageView* profileImage, int pos) {
        if (leaderboardTweeningService) {
            leaderboardTweeningService->CreateImageViewFade("avatar " + std::to_string(pos), 0.0f, 1.0f, 0.5f, profileImage);
        }
    }

    void ProfilePictureView::OnSoftRestart() {
        initializedGlobals = false;
        nullSprite = nullptr;
        mat_UINoGlowRoundEdge = nullptr;
        if (uiMaterials)
        {
            uiMaterials->Reset();
        }
        uiMaterials = nullptr;
        remoteImageService = nullptr;
        leaderboardTweeningService = nullptr;
    }

    ProfilePictureView::ProfilePictureView(HMUI::ImageView* profileImage, UnityEngine::GameObject* loadingIndicator) : profileImage(profileImage), loadingIndicator(loadingIndicator) { }

    void ProfilePictureView::Parsed() {
        if (!initializedGlobals) {
            uiMaterials = BSML::Helpers::GetDiContainer()->Resolve<SnoreSaber::Core::Presentation::SnoreSaberUIMaterials*>();
            remoteImageService = BSML::Helpers::GetDiContainer()->Resolve<SnoreSaber::Core::Presentation::RemoteImageService*>();
            leaderboardTweeningService = BSML::Helpers::GetDiContainer()->TryResolve<SnoreSaber::Features::Leaderboards::Services::LeaderboardTweeningService*>();
            nullSprite = uiMaterials->BlankSprite();
            mat_UINoGlowRoundEdge = uiMaterials->RoundedImageMaterial();
            initializedGlobals = true;
        }

        profileImage->material = mat_UINoGlowRoundEdge.ptr();
        profileImage->sprite = nullSprite.ptr();
        profileImage->gameObject->SetActive(true);
        loadingIndicator->gameObject->SetActive(false);
    }

    void ProfilePictureView::SetProfileImage(string url, int pos, CancellationToken cancellationToken) {
        if (!profileImage.isAlive() || !loadingIndicator.isAlive()) {
            return;
        }

        if (cancellationToken.IsCancellationRequested || !remoteImageService) {
            ClearSprite();
            return;
        }

        loadingIndicator->gameObject->SetActive(true);

        auto profileImageSafe = profileImage;
        auto loadingIndicatorSafe = loadingIndicator;
        auto onSuccess = [profileImageSafe, loadingIndicatorSafe, pos, cancellationToken](Sprite* sprite) mutable {
            if (cancellationToken.IsCancellationRequested || !profileImageSafe.isAlive() || !loadingIndicatorSafe.isAlive()) {
                return;
            }

            profileImageSafe->gameObject->SetActive(true);
            profileImageSafe->sprite = sprite;
            loadingIndicatorSafe->gameObject->SetActive(false);
            FadeInAvatar(profileImageSafe.ptr(), pos);
        };
        auto onFailure = [profileImageSafe, loadingIndicatorSafe, cancellationToken](string) mutable {
            if (cancellationToken.IsCancellationRequested || !profileImageSafe.isAlive() || !loadingIndicatorSafe.isAlive()) {
                return;
            }

            profileImageSafe->sprite = nullSprite.ptr();
            loadingIndicatorSafe->gameObject->SetActive(false);
        };
        remoteImageService->LoadSprite(url, onSuccess, onFailure, cancellationToken);
    }

    void ProfilePictureView::ClearSprite() {
        if (profileImage.isAlive()) {
            profileImage->sprite = nullSprite.ptr();
        }
        if (loadingIndicator.isAlive()) {
            loadingIndicator->gameObject->SetActive(false);
        }
    }
}
