#pragma once

#include "Utils/Event.hpp"

#include <HMUI/ViewController.hpp>
#include <TMPro/TextMeshProUGUI.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>
#include <string>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::ViewControllers::CodeEntry, CompeteCodeEntryViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, joinCodeText);
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<TMPro::TextMeshProUGUI>, statusText);

    DECLARE_INSTANCE_METHOD(StringW, get_joinCode);
    DECLARE_INSTANCE_METHOD(void, set_joinCode, StringW value);
    DECLARE_INSTANCE_METHOD(void, CodeEntered, StringW value);
    DECLARE_INSTANCE_METHOD(void, PasteCode);
    DECLARE_INSTANCE_METHOD(void, JoinCode);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    Utils::Event<const std::string&> JoinRequested;

    void Reset();
    void SetStatus(const std::string& value);

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
    std::string _joinCode;
    std::string _status;

    void SetJoinCode(const std::string& value);
    void ApplyJoinCodeDisplay();
    void ApplyStatus();
};
