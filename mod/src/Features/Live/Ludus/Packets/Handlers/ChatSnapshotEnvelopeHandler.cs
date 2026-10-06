using SaberRank.Features.Live.Ludus.Services;
using SaberRank.Features.Live.Protocol;

namespace SaberRank.Features.Live.Ludus.Packets.Handlers {
    internal sealed class ChatSnapshotEnvelopeHandler<TSession> : ILudusEnvelopeHandler<TSession>
        where TSession : ILudusSessionPacketContext {
        private readonly LudusChatMessageBuffer _messages;

        internal ChatSnapshotEnvelopeHandler(LudusChatMessageBuffer messages) {
            _messages = messages;
        }

        public LudusEnvelopeType Type => LudusEnvelopeType.ChatSnapshot;

        public void Handle(TSession session, DecodedLudusEnvelope envelope) {
            _messages.Replace(envelope.ChatSnapshot, session.CurrentLudusMatchId);
            session.NotifyChatMessagesChanged(_messages.MessagesFor(session.CurrentLudusMatchId));
        }
    }
}
