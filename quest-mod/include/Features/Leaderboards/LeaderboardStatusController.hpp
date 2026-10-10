#pragma once

#include "Features/Leaderboards/LeaderboardPresentationController.hpp"
#include "Features/ScoreSubmission/Domain/ScoreUploadResult.hpp"

#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards, LeaderboardStatusController, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(SnoreSaber::Features::Leaderboards::LeaderboardPresentationController*, _presentationController);
    DECLARE_CTOR(ctor, SnoreSaber::Features::Leaderboards::LeaderboardPresentationController* presentationController);

  public:
    void ApplySubmissionStatus(SnoreSaber::Data::Private::ScoreSubmissionStatus status);
};
