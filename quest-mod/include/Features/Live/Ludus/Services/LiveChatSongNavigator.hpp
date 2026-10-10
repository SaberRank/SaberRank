#pragma once

#include "Features/Live/Cancellation.hpp"
#include "Features/Live/Compete/Domain/CompeteSongSelection.hpp"

#include <custom-types/shared/macros.hpp>

#include <memory>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Live::Ludus::Services, LiveChatSongNavigator, System::Object) {
    DECLARE_CTOR(ctor);

  public:
    // blocking; call off the main thread
    bool TryFocusSong(const std::shared_ptr<Compete::Domain::CompeteSongSelection>& selection, const CancellationToken& cancellationToken);
};
