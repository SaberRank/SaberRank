using SnoreSaber.Features.Leaderboards.Domain;
using SnoreSaber.Core.Presentation;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using SnoreSaber.Features.Leaderboards.Services;

namespace SnoreSaber.Features.Leaderboards.UI.Avatars {
    internal class LeaderboardAvatarHost {
        private const int MaximumAvatars = 10;

        internal List<LeaderboardAvatarView> Avatars { get; }

        public LeaderboardAvatarHost(RemoteImageService remoteImageService, SnoreSaberUIMaterials materials, LeaderboardTweeningService leaderboardTweeningService) {
            Avatars = Enumerable.Range(0, MaximumAvatars).Select(index => new LeaderboardAvatarView(index, remoteImageService, materials, leaderboardTweeningService)).ToList();
        }

        internal void LoadAvatars(LeaderboardMap leaderboard, CancellationToken cancellationToken) {
            ClearAvatars();
            int count = Math.Min(leaderboard.Scores.Length, Avatars.Count);
            for (int i = 0; i < count; i++) {
                Avatars[i].Load(leaderboard.Scores[i].Score.Player.Avatar, cancellationToken);
            }
        }

        internal void ClearAvatars() {
            Avatars.ForEach(avatar => avatar.Clear());
        }
    }
}
