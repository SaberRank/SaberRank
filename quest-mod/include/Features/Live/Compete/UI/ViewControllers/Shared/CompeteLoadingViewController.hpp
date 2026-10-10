#pragma once

#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Shared, CompeteLoadingViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, loadingText);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    void SetMessage(const std::string& message, bool pending = true);

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    std::string _message = "Loading...";

    void ApplyMessage();
};
