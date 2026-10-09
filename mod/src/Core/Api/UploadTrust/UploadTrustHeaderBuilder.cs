using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Text;

namespace SnoreSaber.Core.Api.UploadTrust {
    internal static class UploadTrustHeaderBuilder {
        internal static Dictionary<string, string> BuildUploadHeaders(
            string sessionId,
            string sessionKey,
            string playerId,
            string uploadVersionHash,
            string encryptedData,
            byte[] replay,
            UploadTrustSession trust,
            long timestamp) {

            return new Dictionary<string, string> {
                { "x-session-key", sessionKey },
                { "x-session-id", sessionId },
                { "x-snoresaber-upload", "v1" }
            };
        }

        internal static string BuildCanonicalString(
            string buildId,
            string sessionId,
            string playerId,
            string uploadVersionHash,
            string encryptedDataSha256,
            string replaySha256,
            string timestamp,
            string nonce) {

            return string.Join("\n", new[] {
                UploadTrustSession.ProtocolHeaderValue,
                buildId,
                sessionId,
                playerId,
                uploadVersionHash,
                encryptedDataSha256,
                replaySha256,
                timestamp,
                nonce
            });
        }

        internal static string Sha256Hex(string value) {
            return Sha256Hex(Encoding.UTF8.GetBytes(value ?? string.Empty));
        }

        internal static string Sha256Hex(byte[] value) {
            using (var sha256 = SHA256.Create()) {
                return ToLowerHex(sha256.ComputeHash(value ?? new byte[0]));
            }
        }

        internal static string HmacSha256Hex(string credential, string canonicalString) {
            byte[] key = Encoding.UTF8.GetBytes(credential ?? string.Empty);
            byte[] body = Encoding.UTF8.GetBytes(canonicalString ?? string.Empty);
            using (var hmac = new HMACSHA256(key)) {
                return ToLowerHex(hmac.ComputeHash(body));
            }
        }

        private static string CreateNonce() {
            byte[] bytes = new byte[16];
            using (var random = RandomNumberGenerator.Create()) {
                random.GetBytes(bytes);
            }
            return ToLowerHex(bytes);
        }

        private static string ToLowerHex(byte[] bytes) {
            var builder = new StringBuilder(bytes.Length * 2);
            foreach (byte value in bytes) {
                builder.Append(value.ToString("x2"));
            }
            return builder.ToString();
        }
    }
}
