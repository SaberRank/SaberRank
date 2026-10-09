using System.Linq;
using System.Reflection;

namespace SnoreSaber.Core {
    internal static class SnoreSaberEndpoints {
        private const string DefaultWebsiteBaseUrl = "https://snoresaber.vercel.app";
        private const string DefaultApiBaseUrl = "https://snoresaber.vercel.app";
        private const string DefaultCdnBaseUrl = "https://cdn.saberrank.local";
        private const string DefaultLudusUrl = "wss://ludus-1.saberrank.local/v1/connect";

        internal static readonly string WebsiteBaseUrl = ConfiguredUrl("SnoreSaberWebsiteBaseUrl", DefaultWebsiteBaseUrl);
        internal static readonly string ApiBaseUrl = ConfiguredUrl("SnoreSaberApiBaseUrl", DefaultApiBaseUrl);
        internal static readonly string CdnBaseUrl = ConfiguredUrl("SnoreSaberCdnBaseUrl", DefaultCdnBaseUrl);
        internal static readonly string LudusUrl = ConfiguredUrl("SnoreSaberLudusUrl", DefaultLudusUrl);

        internal static string GlobalLeaderboard() {
            return $"{WebsiteBaseUrl}/global";
        }

        internal static string Leaderboard(int leaderboardId) {
            return $"{WebsiteBaseUrl}/leaderboard/{leaderboardId}";
        }

        internal static string Player(string playerId) {
            return $"{WebsiteBaseUrl}/u/{playerId}";
        }

        internal static string Flag(string country) {
            return $"{CdnBaseUrl}/flags/{country.ToLower()}.png";
        }

        private static string ConfiguredUrl(string key, string fallback) {
            string value = typeof(Plugin)
                .Assembly
                .GetCustomAttributes<AssemblyMetadataAttribute>()
                .FirstOrDefault(x => x.Key == key)
                ?.Value;

            return string.IsNullOrWhiteSpace(value) ? fallback : value.Trim().TrimEnd('/');
        }
    }
}
