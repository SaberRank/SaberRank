using SnoreSaber.Features.Replays.Format;
using System.Threading.Tasks;

namespace SnoreSaber.Features.Replays {
    internal class ReplayFileCodec {
        internal Task<ReplayFile> Read(byte[] replay) {
            return Task.Run(() => new ReplayFileReader().Read(replay));
        }

        internal Task<byte[]> Write(ReplayFile replay) {
            return Task.Run(() => new ReplayFileWriter().Write(replay));
        }
    }
}
