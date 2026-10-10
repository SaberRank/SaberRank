#pragma once

#include <Zenject/IInitializable.hpp>
#include <System/IDisposable.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <cstdint>
#include <memory>

namespace SnoreSaber::Core::Timing::Detail
{
    struct ClockState;
}

DECLARE_CLASS_CODEGEN_INTERFACES(
    SnoreSaber::Core::Timing,
    SnoreSaberClock,
    System::Object,
    Zenject::IInitializable*,
    System::IDisposable*) {
    DECLARE_CTOR(ctor);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Initialize, &::Zenject::IInitializable::Initialize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);

  public:
    int64_t UnixTimeMilliseconds();
    int64_t MonotonicMilliseconds();
    void RecordLudusServerTime(int64_t serverTimeUnixMs);

  private:
    // shared with the sync thread; the thread never touches the managed object
    std::shared_ptr<Detail::ClockState> _state;
};
