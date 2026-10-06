using SaberRank.Features.Live.Ludus.Services;
using SaberRank.Features.Live.Ludus.Packets;
using SaberRank.Features.Live.Protocol;

namespace SaberRank.Features.Live.Compete.Packets.Handlers {
    internal sealed class ServerCommandEnvelopeHandler : ILudusEnvelopeHandler<ILudusSessionPacketContext> {
        private readonly LudusServerCommandDispatcher _commandDispatcher;
        private readonly ILudusServerCommandSession _commandSession;

        internal ServerCommandEnvelopeHandler(LudusServerCommandDispatcher commandDispatcher, ILudusServerCommandSession commandSession) {
            _commandDispatcher = commandDispatcher;
            _commandSession = commandSession;
        }

        public LudusEnvelopeType Type => LudusEnvelopeType.ServerCommand;

        public void Handle(ILudusSessionPacketContext session, DecodedLudusEnvelope envelope) {
            _commandDispatcher.Handle(_commandSession, envelope.ServerCommand);
        }
    }
}
