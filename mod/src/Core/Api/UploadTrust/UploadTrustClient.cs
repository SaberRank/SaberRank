using SnoreSaber.Core.Api.Generated;
using System;

namespace SnoreSaber.Core.Api.UploadTrust {
    internal sealed class UploadTrustClient {
        private readonly SnoreSaberRuntimeInfo _runtimeInfo;
        private readonly UploadTrustBuildMetadata _buildMetadata;

        internal UploadTrustClient(SnoreSaberRuntimeInfo runtimeInfo)
            : this(runtimeInfo, UploadTrustBuildMetadata.FromAssembly(typeof(Plugin).Assembly)) {
        }

        internal UploadTrustClient(SnoreSaberRuntimeInfo runtimeInfo, UploadTrustBuildMetadata buildMetadata) {
            _runtimeInfo = runtimeInfo;
            _buildMetadata = buildMetadata;
        }

        internal void ApplyAuthMetadata(GameAuthenticateRequest request) {
            // SnoreSaber game uploads are authenticated by the game session.
            // We intentionally do not send or require ScoreSaber-style upload trust metadata.
        }

        internal UploadTrustSession CreateSession(GameAuthenticateResponse response) {
            // Uploads no longer depend on build trust. The game session returned by
            // /game/auth is the only credential needed for SnoreSaber score uploads.
            return null;
        }
    }
}
