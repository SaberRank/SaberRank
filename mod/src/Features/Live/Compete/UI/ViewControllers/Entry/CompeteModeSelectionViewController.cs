using BeatSaberMarkupLanguage.Attributes;
using BeatSaberMarkupLanguage.ViewControllers;
using System;

namespace SaberRank.Features.Live.Compete.UI.ViewControllers.Entry {
    [HotReload]
    internal class CompeteModeSelectionViewController : BSMLAutomaticViewController {
        internal event Action BrowserSelected;
        internal event Action JoinViaCodeSelected;

        [UIAction("select-browser")]
        private void SelectBrowser() {
            BrowserSelected?.Invoke();
        }

        [UIAction("select-join-code")]
        private void SelectJoinViaCode() {
            JoinViaCodeSelected?.Invoke();
        }
    }
}
