using System;
using System.Net.Http;
using System.Reflection;

namespace SaberRank.Core.Api.Generated {

    public partial class SaberRankApiGeneratedClient {
        internal string UserAgent { get; set; }

        partial void Initialize() {
            string version = Assembly.GetExecutingAssembly().GetName().Version.ToString();
            UserAgent = $"SaberRank-PC/{version}";
        }

        partial void PrepareRequest(HttpClient client, HttpRequestMessage request, string url) {
            if (!string.IsNullOrEmpty(UserAgent)) {
                request.Headers.TryAddWithoutValidation("User-Agent", UserAgent);
            }
        }
    }
}
