namespace SaberRank.Core.Api {
    internal class SaberRankApiError {
        internal int StatusCode { get; set; }
        internal bool NetworkError { get; set; }
        internal string Code { get; set; } = string.Empty;
        internal string Message { get; set; } = string.Empty;
        internal string RawBody { get; set; } = string.Empty;

        internal static SaberRankApiError FromMessage(string message) {
            return new SaberRankApiError {
                Message = message
            };
        }
    }
}
