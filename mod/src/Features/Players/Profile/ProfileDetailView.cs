using BeatSaberMarkupLanguage;
using BeatSaberMarkupLanguage.Attributes;
using HMUI;
using SnoreSaber.Core.Presentation;
using SnoreSaber.Core;
using SnoreSaber.Features.Leaderboards.UI;
using SnoreSaber.Features.Players.Services;
using SnoreSaber.Features.Players.Domain;
using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using UnityEngine;
using Zenject;

namespace SnoreSaber.Features.Players.Profile {

    internal class ProfileDetailView : MonoBehaviour, INotifyPropertyChanged {

        public event PropertyChangedEventHandler PropertyChanged;

        #region BSML Components
        [UIComponent("profile-modal-root")]
        public ModalView profileModalRoot = null;

        [UIComponent("profile-top")]
        protected ImageView _profileTop = null;

        [UIComponent("profile-line-border")]
        protected ImageView _profileLineBorder = null;

        [UIComponent("profile-picture")]
        public readonly ImageView profilePicture = null;

        [UIComponent("profile-prefix-picture")]
        protected readonly ImageView _profilePrefixPicture = null;
        public string profilePrefixPicture {
            set {
                bool hasImage = value != null;
                _profilePrefixPicture.gameObject.SetActive(hasImage);
                if (hasImage) {
                    _profilePrefixPicture.SetImageAsync(value).RunTask();
                }
            }
        }

        [UIComponent("player-name-text")]
        public readonly CurvedTextMeshPro playerNameText = null;

        [UIComponent("rank-text")]
        public readonly CurvedTextMeshPro rankText = null;

        [UIComponent("pp-text")]
        public readonly CurvedTextMeshPro ppText = null;

        [UIComponent("ranked-acc-text")]
        public readonly CurvedTextMeshPro rankedAccText = null;

        [UIComponent("total-score-text")]
        public readonly CurvedTextMeshPro totalScoreText = null;
        #endregion

        #region BSML Values
        private readonly ProfileBadgeHost _badgeHost = new ProfileBadgeHost();
        [UIValue("badge-host")]
        protected ProfileBadgeHost badgeHost => _badgeHost;

        private bool _profileSet = false;
        [UIValue("profile-set")]
        public bool profileSet {
            get => _profileSet;
            set {
                _profileSet = value;
                NotifyPropertyChanged();
            }
        }
        private bool _profileSetLoading = false;
        [UIValue("profile-set-loading")]
        public bool profileSetLoading {
            get => _profileSetLoading;
            set {
                _profileSetLoading = value;
                NotifyPropertyChanged();
            }
        }
        #endregion

        #region Custom Properties
        private ProfileDetailData _profileInfo { get; set; }
        private bool _isCyan { get; set; }

        private readonly HoverHint _profileHoverHint = null;
        private HoverHint profileHoverHint => _profileHoverHint ?? _profilePrefixPicture.gameObject.GetComponent<HoverHint>();
        #endregion

        private PlayerProfileService _playerProfileService = null;
        private SnoreSaberUIMaterials _materials = null;
        private IDisposable _hotReload;

        [Inject]
        private void Construct(PlayerProfileService playerProfileService, SnoreSaberUIMaterials materials) {
            _playerProfileService = playerProfileService;
            _materials = materials;
            ApplyRoundedMaterials();
        }

        internal void UseHotReload(IDisposable hotReload) {
            _hotReload?.Dispose();
            _hotReload = hotReload;
        }

        private void OnDestroy() => _hotReload?.Dispose();

        [UIAction("profile-url-click")]
        private void ProfileURLClicked() {
            if (_profileInfo == null) {
                return;
            }

            Application.OpenURL(SnoreSaberEndpoints.Player(_profileInfo.Player.Id));
        }

        [UIAction("#post-parse")]
        protected void Parsed() {
            _profileTop.material = Utilities.ImageResources.NoGlowMat;
            var background = profileModalRoot.gameObject.transform.GetChild(0);
            background.gameObject.SetActive(false);

            var modalPic = profilePicture;
            PanelView.ImageSkew(ref modalPic) = 0f;
            PanelView.ImageSkew(ref _profileLineBorder) = 0f;
            PanelView.ImageSkew(ref _profileTop) = 0f;

            ApplyRoundedMaterials();
        }

        internal async Task ShowProfile(string playerId) {
            ApplyCrown(null);
            SetLoadingState(true);

            try {
                if (string.IsNullOrWhiteSpace(playerId)) {
                    throw new InvalidOperationException("SnoreSaber profile was requested without a player ID.");
                }

                Plugin.Log.Debug($"Loading SnoreSaber profile for player {playerId}.");
                PlayerProfile player = await _playerProfileService.GetPlayerInfo(playerId, full: true);
                if (player == null) {
                    throw new InvalidOperationException($"SnoreSaber returned no profile for player {playerId}.");
                }

                _profileInfo = ProfileDetailData.Create(player);

                await ApplyProfileFont(_profileInfo);

                playerNameText.text = _profileInfo.DisplayName;
#pragma warning disable CS0612 // Type or member is obsolete
                profilePicture.SetImage(_profileInfo.Avatar);
#pragma warning restore CS0612 // Type or member is obsolete

                rankText.text = _profileInfo.RankText;
                ppText.text = _profileInfo.PPText;
                rankedAccText.text = _profileInfo.RankedAccuracyText;
                totalScoreText.text = _profileInfo.TotalScoreText;

                SetProfileBadges(_profileInfo.Badges);
                ApplyCrown(_profileInfo.Crown);
                SetLoadingState(false);
            } catch (Exception ex) {
                Plugin.Log.Error($"Failed to load SnoreSaber profile for {playerId}: {ex}");
                playerNameText.text = "Profile unavailable";
                rankText.text = "#0";
                ppText.text = "<color=#6772E5>0pp</color>";
                rankedAccText.text = "--";
                totalScoreText.text = "--";
                SetProfileBadges(new System.Collections.Generic.List<ProfileBadgeData>());
                SetLoadingState(false);
            }
        }

        public void SetProfileBadges(System.Collections.Generic.IReadOnlyList<ProfileBadgeData> badges) => _badgeHost.SetBadges(badges);

        public void SetLoadingState(bool loading) {
            profileSet = !loading;
            profileSetLoading = loading;
        }

        private async Task ApplyProfileFont(ProfileDetailData profile) {

            if (profile.UsesFurryFont) {
                var mat = await _materials.GetFurryFontMaterial();
                playerNameText.fontMaterial = mat;
                _isCyan = true;
                return;
            }
            if (_isCyan) {
                playerNameText.fontMaterial = _materials.DefaultFontMaterial;
            }
        }

        private void ApplyRoundedMaterials() {
            if (_materials == null || profilePicture == null) {
                return;
            }

            var modalPic = profilePicture;
            modalPic.material = _materials.RoundedImageMaterial;
        }

        private void ApplyCrown(ProfileCrownData crown) {

            profilePrefixPicture = null;
            bool hasCrown = crown != null && crown.HasCrown;
            profileHoverHint.enabled = hasCrown;

            if (!hasCrown) {
                return;
            }

            profilePrefixPicture = crown.Image;
            profileHoverHint.text = crown.Description;
        }

        protected void NotifyPropertyChanged([CallerMemberName] string propertyName = "") => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
