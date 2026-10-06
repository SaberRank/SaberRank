using System.Linq;
using System.Reflection;

namespace SaberRank.Core.Api.UploadTrust {
    internal sealed class UploadTrustBuildMetadata {
        private const string BuildIdKey = "SaberRankOfficialBuildId";
        private const string CredentialKey = "SaberRankOfficialBuildCredential";
        private const string ArtifactSha256Key = "SaberRankOfficialArtifactSha256";
        private const string DevelopmentUploadTokenKey = "SaberRankDevelopmentUploadToken";
        private const string DevelopmentAuthNonceKey = "SaberRankDevelopmentAuthNonce";
        private const string DevelopmentPlayerIdKey = "SaberRankDevelopmentPlayerId";
        private const string DevelopmentPlayerNameKey = "SaberRankDevelopmentPlayerName";

        internal UploadTrustBuildMetadata(
            string buildId,
            string credential,
            string artifactSha256,
            string developmentUploadToken,
            string developmentAuthNonce,
            string developmentPlayerId,
            string developmentPlayerName) {

            BuildId = buildId ?? string.Empty;
            Credential = credential ?? string.Empty;
            ArtifactSha256 = artifactSha256 ?? string.Empty;
            DevelopmentUploadToken = developmentUploadToken ?? string.Empty;
            DevelopmentAuthNonce = developmentAuthNonce ?? string.Empty;
            DevelopmentPlayerId = developmentPlayerId ?? string.Empty;
            DevelopmentPlayerName = developmentPlayerName ?? string.Empty;
        }

        internal string BuildId { get; }
        internal string Credential { get; }
        internal string ArtifactSha256 { get; }
        internal string DevelopmentUploadToken { get; }
        internal string DevelopmentAuthNonce { get; }
        internal string DevelopmentPlayerId { get; }
        internal string DevelopmentPlayerName { get; }

        internal bool IsOfficial => !string.IsNullOrEmpty(BuildId) && !string.IsNullOrEmpty(Credential);
        internal bool IsDevelopment => !IsOfficial && !string.IsNullOrEmpty(DevelopmentUploadToken);
        internal bool HasDevelopmentAuth => !string.IsNullOrEmpty(DevelopmentAuthNonce);

        internal static UploadTrustBuildMetadata FromAssembly(Assembly assembly) {
            var metadata = assembly.GetCustomAttributes<AssemblyMetadataAttribute>().ToList();
            return new UploadTrustBuildMetadata(
                GetValue(metadata, BuildIdKey),
                GetValue(metadata, CredentialKey),
                GetValue(metadata, ArtifactSha256Key),
                GetValue(metadata, DevelopmentUploadTokenKey),
                GetValue(metadata, DevelopmentAuthNonceKey),
                GetValue(metadata, DevelopmentPlayerIdKey),
                GetValue(metadata, DevelopmentPlayerNameKey));
        }

        private static string GetValue(System.Collections.Generic.List<AssemblyMetadataAttribute> metadata, string key) {
            AssemblyMetadataAttribute attribute = metadata.FirstOrDefault(x => x.Key == key);
            return attribute == null ? string.Empty : attribute.Value;
        }
    }
}
