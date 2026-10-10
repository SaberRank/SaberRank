#include "Features/MainMenu/MainFlow/SnoreSaberFlowCoordinator.hpp"
#include <HMUI/ViewController.hpp>
#include <bsml/shared/Helpers/creation.hpp>

DEFINE_TYPE(SnoreSaber::UI::FlowCoordinators, SnoreSaberFlowCoordinator);

using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace HMUI;
using namespace BSML::Helpers;

namespace SnoreSaber::UI::FlowCoordinators
{
    void SnoreSaberFlowCoordinator::ctor(SnoreSaber::UI::ViewControllers::FAQViewController* faqViewController,
                                         SnoreSaber::UI::ViewControllers::TeamViewController* teamViewController,
                                         SnoreSaber::UI::ViewControllers::GlobalViewController* globalViewController)
    {
        INVOKE_CTOR();
        this->faqViewController = faqViewController;
        this->teamViewController = teamViewController;
        this->globalViewController = globalViewController;
    }

    void SnoreSaberFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            if (!globalViewController)
            {
                globalViewController = CreateViewController<SnoreSaber::UI::ViewControllers::GlobalViewController*>();
            }
            if (!faqViewController)
            {
                faqViewController = CreateViewController<SnoreSaber::UI::ViewControllers::FAQViewController*>();
            }
            if (!teamViewController)
            {
                teamViewController = CreateViewController<SnoreSaber::UI::ViewControllers::TeamViewController*>();
            }

            SetTitle("SnoreSaber", ViewController::AnimationType::Out);
            showBackButton = true;
            ProvideInitialViewControllers(globalViewController, teamViewController, faqViewController, nullptr, nullptr);
        }
        // HACK: if we don't do this the viewcontroller remains active when returning to the main song menu (don't ask me why)
        faqViewController->gameObject->SetActive(true);
    }

    void SnoreSaberFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController)
    {
        // HACK: if we don't do this the viewcontroller remains active when returning to the main song menu (don't ask me why)
        faqViewController->gameObject->SetActive(false);
        HMUI::FlowCoordinator* flowCoordinator = nullptr;
        if (presentingFlowCoordinator)
        {
            flowCoordinator = presentingFlowCoordinator.unsafePtr();
        }
        else if (this->_parentFlowCoordinator)
        {
            flowCoordinator = this->_parentFlowCoordinator.unsafePtr();
        }
        if (flowCoordinator)
        {
            flowCoordinator->DismissFlowCoordinator(this, ViewController::AnimationDirection::Horizontal, nullptr, false);
        }
    }

    void SnoreSaberFlowCoordinator::SetPresentingFlowCoordinator(HMUI::FlowCoordinator* flowCoordinator)
    {
        presentingFlowCoordinator = flowCoordinator;
    }
}
