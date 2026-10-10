#pragma once

#include <HMUI/FlowCoordinator.hpp>
#include <HMUI/ViewController.hpp>
#include "Features/MainMenu/MainFlow/FAQ/FAQViewController.hpp"
#include "Features/MainMenu/MainFlow/GlobalLeaderboard/GlobalViewController.hpp"
#include "Features/MainMenu/MainFlow/Teams/UI/TeamViewController.hpp"
#include <UnityEngine/GameObject.hpp>
#include <custom-types/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::UI::FlowCoordinators, SnoreSaberFlowCoordinator, HMUI::FlowCoordinator) {
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::FlowCoordinator::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_OVERRIDE_METHOD_MATCH(void, BackButtonWasPressed, &HMUI::FlowCoordinator::BackButtonWasPressed, HMUI::ViewController* topViewController);

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityW<SnoreSaber::UI::ViewControllers::GlobalViewController>, globalViewController, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityW<SnoreSaber::UI::ViewControllers::FAQViewController>, faqViewController, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityW<SnoreSaber::UI::ViewControllers::TeamViewController>, teamViewController, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityW<HMUI::FlowCoordinator>, presentingFlowCoordinator, nullptr);

    DECLARE_CTOR(ctor,
                 SnoreSaber::UI::ViewControllers::FAQViewController* faqViewController,
                 SnoreSaber::UI::ViewControllers::TeamViewController* teamViewController,
                 SnoreSaber::UI::ViewControllers::GlobalViewController* globalViewController);

  public:
    void SetPresentingFlowCoordinator(HMUI::FlowCoordinator* flowCoordinator);
};
