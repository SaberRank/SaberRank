using BeatSaberMarkupLanguage.Attributes;
using HMUI;
using SnoreSaber.Core;
using SnoreSaber.Core.Presentation;
using System;

namespace SnoreSaber.Features.MainMenu.MainFlow.GlobalLeaderboard {
    internal class GlobalCell {

        #region BSML Components
        [UIComponent("profile-image")]
        private readonly ImageView _imageView = null;
        #endregion

        #region BSML Values
        [UIValue("pfp-url")]
        private readonly string _avatarUrl;

        [UIValue("username")]
        private readonly string _username;

        [UIValue("rank")]
        private readonly string _globalRank;

        [UIValue("pp")]
        private readonly string _ppText;

        [UIValue("flag-url")]
        private readonly string _flagUrl;

        [UIValue("country")]
        private readonly string _countryText;
        #endregion

        private readonly string _identifier;
        private readonly Action<string, string> _profileClicked;
        private readonly SnoreSaberUIMaterials _materials;

        public GlobalCell(SnoreSaberUIMaterials materials, string id, string avatarUrl, string username, string country, string rank, double pp, Action<string, string> onActivateProfile = null) {

            _materials = materials;
            _identifier = id;
            _avatarUrl = avatarUrl;
            _ppText = string.Format("<color=#6772E5>{0:n0} ZZ</color>", pp);
            _username = username;
            _globalRank = rank;
            _profileClicked = onActivateProfile;
            _countryText = $"{country}";
            _flagUrl = SnoreSaberEndpoints.Flag(country);
        }

        [UIAction("profile-clicked")]
        private void ProfileClicked() {

            _profileClicked?.Invoke(_identifier, _username);
        }

        [UIAction("#post-parse")]
        private void Parsed() {

            _imageView.material = _materials.RoundedImageMaterial;
        }
    }
}
