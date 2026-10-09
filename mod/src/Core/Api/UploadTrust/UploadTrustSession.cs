namespace SnoreSaber.Core.Api.UploadTrust {
    internal sealed class UploadTrustSession {
        internal const string ProtocolHeaderValue = "snoresaber-upload-v1";
        internal const int ProtocolVersion = 1;

        internal UploadTrustSession(
            string buildId,
            string buildCredential,
            string uploadVersionHash,
            bool requiresBuildId) {

            BuildId = buildId ?? string.Empty;
            BuildCredential = buildCredential ?? string.Empty;
            UploadVersionHash = uploadVersionHash ?? string.Empty;
            RequiresBuildId = requiresBuildId;
        }

        internal string BuildId { get; }
        internal string BuildCredential { get; }
        internal string UploadVersionHash { get; }
        internal bool RequiresBuildId { get; }

        internal bool IsUploadProtocolV2 {
            get {
                return (!RequiresBuildId || !string.IsNullOrEmpty(BuildId)) &&
                    !string.IsNullOrEmpty(BuildCredential) &&
                    !string.IsNullOrEmpty(UploadVersionHash);
            }
        }
    }
}
