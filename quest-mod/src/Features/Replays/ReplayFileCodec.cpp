#include "Features/Replays/ReplayFileCodec.hpp"

#include "Features/Replays/Format/ReplayReader.hpp"
#include "Features/Replays/Format/ReplayWriter.hpp"

namespace SnoreSaber::ReplaySystem::ReplayFileCodec
{
    std::shared_ptr<SnoreSaber::Data::Private::ReplayFile> Read(const std::vector<char>& replay)
    {
        return SnoreSaber::Data::Private::ReplayReader::Read(replay);
    }

    std::vector<char> Write(const std::shared_ptr<SnoreSaber::Data::Private::ReplayFile>& replay)
    {
        return SnoreSaber::Data::Private::ReplayWriter::Write(replay);
    }
}
