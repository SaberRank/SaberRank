#pragma once
#include <HMUI/ImageView.hpp>
#include <HMUI/ViewController.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::ViewControllers, FAQViewController, HMUI::ViewController) {
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate,
                            &HMUI::ViewController::DidActivate,
                            bool firstActivation, bool addedToHierarchy,
                            bool screenSystemEnabling);

    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, scoreSaberImage);
    DECLARE_INSTANCE_FIELD(UnityW<HMUI::ImageView>, bsmgImage);

    void SnoreSaberImageClicked();
    void BsmgImageClicked();

private:
    int scoreSaberCounter = 0;
    int bsmgCounter = 0;
};
