#include "Features/Live/Compete/UI/ViewControllers/Shared/CompeteLoadingViewController.hpp"

#include "assets.hpp"

#include <bsml/shared/BSML.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Shared, CompeteLoadingViewController);

namespace SnoreSaber::Features::Live::Compete::UI::ViewControllers::Shared
{
    void CompeteLoadingViewController::ctor()
    {
        INVOKE_CTOR();
    }

    void CompeteLoadingViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::CompeteLoadingViewController_bsml, transform, this);
        }

        ApplyMessage();
    }

    void CompeteLoadingViewController::SetMessage(const std::string& message, bool pending)
    {
        if (pending && !message.ends_with("..."))
        {
            _message = message + "...";
        }
        else
        {
            _message = message;
        }

        ApplyMessage();
    }

    void CompeteLoadingViewController::ApplyMessage()
    {
        if (loadingText)
        {
            loadingText->text = _message;
        }
    }
}
