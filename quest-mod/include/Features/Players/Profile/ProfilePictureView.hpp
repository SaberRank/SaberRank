#pragma once
#include <HMUI/ImageView.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Sprite.hpp>
#include <System/Threading/CancellationToken.hpp>
#include <beatsaber-hook/shared/utils/typedefs-wrappers.hpp>
#include <string>

namespace SnoreSaber::UI::Other {
    // shouldn't need to be a C# class until we switch over to BSML
    class ProfilePictureView {
        SafePtrUnity<HMUI::ImageView> profileImage;
        SafePtrUnity<UnityEngine::GameObject> loadingIndicator;
    public:
        ProfilePictureView(HMUI::ImageView* profileImage, UnityEngine::GameObject* loadingIndicator);
        void Parsed(); // we are not using BSML here yet, but keep names analogous
        void SetProfileImage(std::string url, int pos, System::Threading::CancellationToken cancellationToken);
        void ClearSprite();

        static void OnSoftRestart();
    };
}
