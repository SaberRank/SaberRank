namespace SnoreSaber.Core.Api {
    internal class SnoreSaberApiError {
        internal int StatusCode { get; set; }
        internal bool NetworkError { get; set; }
        internal string Code { get; set; } = string.Empty;
        internal string Message { get; set; } = string.Empty;
        internal string RawBody { get; set; } = string.Empty;

        internal static SnoreSaberApiError FromMessage(string message) {
            return new SnoreSaberApiError {
                Message = message
            };
        }
    }
}
