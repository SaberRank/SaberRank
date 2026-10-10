#pragma once

#include "Utils/Event.hpp"

#include <HMUI/ViewController.hpp>
#include <bsml/shared/BSML/Parsing/BSMLParser.hpp>
#include <custom-types/shared/macros.hpp>

#include <memory>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Entry, CompeteModeSelectionViewController, HMUI::ViewController) {
    DECLARE_INSTANCE_METHOD(void, SelectBrowser);
    DECLARE_INSTANCE_METHOD(void, SelectJoinViaCode);

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_CTOR(ctor);

  public:
    Utils::Event<> BrowserSelected;
    Utils::Event<> JoinViaCodeSelected;

  private:
    std::shared_ptr<BSML::BSMLParser> _parser;
};
