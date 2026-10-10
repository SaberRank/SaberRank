#include "Features/Live/Compete/UI/ViewControllers/Entry/CompeteModeSelectionViewController.hpp"

#include "assets.hpp"

#include <bsml/shared/BSML.hpp>

DEFINE_TYPE(SnoreSaber::Features::Live::Compete::UI::ViewControllers::Entry, CompeteModeSelectionViewController);

namespace SnoreSaber::Features::Live::Compete::UI::ViewControllers::Entry
{
    void CompeteModeSelectionViewController::ctor()
    {
        INVOKE_CTOR();
    }

    void CompeteModeSelectionViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if (firstActivation)
        {
            _parser = BSML::parse_and_construct(IncludedAssets::CompeteModeSelectionViewController_bsml, transform, this);
        }
    }

    void CompeteModeSelectionViewController::SelectBrowser()
    {
        BrowserSelected.Invoke();
    }

    void CompeteModeSelectionViewController::SelectJoinViaCode()
    {
        JoinViaCodeSelected.Invoke();
    }
}
