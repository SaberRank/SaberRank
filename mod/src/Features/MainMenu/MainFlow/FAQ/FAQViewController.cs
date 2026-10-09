using BeatSaberMarkupLanguage.Attributes;
using BeatSaberMarkupLanguage.ViewControllers;
using HMUI;
using SnoreSaber.Core.Presentation;
using UnityEngine;
using Zenject;

namespace SnoreSaber.Features.MainMenu.MainFlow.FAQ {
    [HotReload]
    internal class FAQViewController : BSMLAutomaticViewController {
        [UIAction("website-clicked")]
        protected void WebsiteClicked() => Application.OpenURL("https://bit.ly/37Zp5Fq");

        [UIAction("discord-clicked")]
        protected void DiscordClicked() => Application.OpenURL("https://bit.ly/350Fd7Y");

        [UIAction("twitter-clicked")]
        protected void TwitterClicked() => Application.OpenURL("https://bit.ly/3b0aN9x");

        [UIAction("patreon-clicked")]
        protected void PatreonClicked() => Application.OpenURL("https://bit.ly/3nXRT6S");

        [UIAction("bsmg-discord-clicked")]
        protected void BSMGDiscordClicked() => Application.OpenURL("https://bit.ly/3pP8F91");

        [UIAction("bsmg-wiki-clicked")]
        protected void BSMGWikiClicked() => Application.OpenURL("https://bit.ly/3rGGsme");

        [UIAction("bsmg-patreon-clicked")]
        protected void BSMGPatreonClicked() => Application.OpenURL("https://bit.ly/34ZRmdb");

        [UIComponent("bsmg-image")]
        protected readonly ImageView _bsmgImageView = null;

        private SnoreSaberUIMaterials _materials = null;

        [Inject]
        internal void Construct(SnoreSaberUIMaterials materials) {
            _materials = materials;
        }

        private string _snoreSaberImage = "SnoreSaber.Resources.logo-large.png";
        [UIValue("snoresaber-image")]
        public string snoreSaberImage {
            get => _snoreSaberImage;
            set {
                _snoreSaberImage = value;
                NotifyPropertyChanged();
            }
        }

        private string _bsmgImage = "SnoreSaber.Resources.bsmg.jpg";
        [UIValue("bsmg-image")]
        public string bsmgImage {
            get => _bsmgImage;
            set {
                _bsmgImage = value;
                NotifyPropertyChanged();
            }
        }

        private int _snoreSaberCounter;
        [UIAction("snoresaber-image-clicked")]
        public void SnoreSaberImageClicked() {

            _snoreSaberCounter++;
            if (_snoreSaberCounter == 5) {
                snoreSaberImage = "SnoreSaber.Resources.logo-flushed.png";
            }
            if (_snoreSaberCounter == 10) {
                snoreSaberImage = "SnoreSaber.Resources.logo-large.png";
                _snoreSaberCounter = 0;
            }
        }

        private int _bsmgCounter;
        [UIAction("bsmg-image-clicked")]
        public void BsmgImageClicked() {

            _bsmgCounter++;
            if (_bsmgCounter == 5) {
                bsmgImage = "SnoreSaber.Resources.cmb.png";
            }
            if (_bsmgCounter == 10) {
                bsmgImage = "SnoreSaber.Resources.cmb-blush.png";
            }
            if (_bsmgCounter == 15) {
                bsmgImage = "SnoreSaber.Resources.bsmg.jpg";
                _bsmgCounter = 0;
            }
        }

        [UIAction("#post-parse")]
        private void Parsed() {
            _bsmgImageView.material = _materials.RoundedImageMaterial;
        }
    }
}
