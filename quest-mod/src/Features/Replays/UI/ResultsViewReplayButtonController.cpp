#include "Core/Gameplay/SnoreSaberGameplayModifiers.hpp"
#include "Features/Replays/UI/ResultsViewReplayButtonController.hpp"
#include "Features/Players/Services/PlayerService.hpp"
#include "Services/ReplayService.hpp"

#include <bsml/shared/BSML.hpp>
#include <custom-types/shared/delegate.hpp>
#include "Utils/AsyncUtils.hpp"
#include "Utils/OperatorOverloads.hpp"
#include "Utils/SafePtr.hpp"
#include "logging.hpp"

using namespace BSML;

DEFINE_TYPE(SnoreSaber::ReplaySystem::UI, ResultsViewReplayButtonController);

namespace SnoreSaber::ReplaySystem::UI
{
    void ResultsViewReplayButtonController::ctor(GlobalNamespace::ResultsViewController* resultsViewController, ReplayLoader* replayLoader)
    {
        INVOKE_CTOR();
        _resultsViewController = resultsViewController;
        _replayLoader = replayLoader;
    }

    void ResultsViewReplayButtonController::Initialize()
    {
        didActivateDelegate = { &ResultsViewReplayButtonController::ResultsViewController_didActivateEvent, this };
        continueButtonPressedDelegate = { &ResultsViewReplayButtonController::ResultsViewController_continueButtonPressedEvent, this };
        restartButtonPressedDelegate = { &ResultsViewReplayButtonController::ResultsViewController_restartButtonPressedEvent, this };

        _resultsViewController->___didActivateEvent += didActivateDelegate;
        _resultsViewController->___continueButtonPressedEvent += continueButtonPressedDelegate;
        _resultsViewController->___restartButtonPressedEvent += restartButtonPressedDelegate;

        SafePtr<ResultsViewReplayButtonController> self(this);
        SnoreSaber::Services::ReplayService::ReplaySerialized = [self](const std::vector<char> &v) { self->UploadDaemon_ReplaySerialized(v); };
    }

    void ResultsViewReplayButtonController::ResultsViewController_didActivateEvent(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
    {
        if(firstActivation) {
            BSML::parse_and_construct(
                "<button-with-icon id=\"watchReplayButton\" icon=\"SnoreSaber_replay_png\" hover-hint=\"Watch Replay\" pref-width=\"15\" pref-height=\"13\" interactable=\"false\" on-click=\"ClickedReplayButton\" />",
                _resultsViewController->gameObject->transform,
                this
            );
            watchReplayButton->transform->localScale = watchReplayButton->transform->localScale * 0.4f;
            watchReplayButton->transform->localPosition = {42.5f, 27.0f, 0.0f};
        }
        SetReplayReady(!_serializedReplay.empty());
        _beatmapLevel = _resultsViewController->_beatmapLevel;
        _beatmapKey = _resultsViewController->_beatmapKey;
        _levelCompletionResults = _resultsViewController->_levelCompletionResults;
    }

    void ResultsViewReplayButtonController::ResultsViewController_restartButtonPressedEvent(UnityW<GlobalNamespace::ResultsViewController> obj)
    {
        _serializedReplay.clear();
        SetReplayReady(false);
    }

    void ResultsViewReplayButtonController::ResultsViewController_continueButtonPressedEvent(UnityW<GlobalNamespace::ResultsViewController> obj)
    {
        _serializedReplay.clear();
        SetReplayReady(false);
    }

    void ResultsViewReplayButtonController::UploadDaemon_ReplaySerialized(const std::vector<char> &serializedReplay)
    {
        SafePtr<ResultsViewReplayButtonController> self(this);
        SnoreSaber::Utils::Async::Main([self, serializedReplay] {
            self->_serializedReplay = serializedReplay;
            self->SetReplayReady(true);
        });
    }

    void ResultsViewReplayButtonController::WaitForReplay()
    {
        SetReplayReady(_replayReady);
    }

    void ResultsViewReplayButtonController::Dispose()
    {
        if(_resultsViewController) {
            _resultsViewController->___didActivateEvent -= didActivateDelegate;
            _resultsViewController->___continueButtonPressedEvent -= continueButtonPressedDelegate;
            _resultsViewController->___restartButtonPressedEvent -= restartButtonPressedDelegate;
        }
        SnoreSaber::Services::ReplayService::ReplaySerialized = nullptr;
    }

    void ResultsViewReplayButtonController::ClickedReplayButton()
    {
        watchReplayButton->interactable = false;

        std::vector<std::string> modifiersVec = SnoreSaber::Core::Gameplay::SnoreSaberGameplayModifiers::ToCodeList(_levelCompletionResults->gameplayModifiers, _levelCompletionResults->energy);
        std::string modifiers;
        for(int i = 0; i < modifiersVec.size(); ++i) {
            if(i > 0) modifiers += ',';
            modifiers += modifiersVec[i];
        }

        SafePtr<ReplayLoader> replayLoader(_replayLoader);
        SafePtr<GlobalNamespace::BeatmapLevel> beatmapLevel(_beatmapLevel);
        auto beatmapKey = _beatmapKey;
        auto serializedReplay = _serializedReplay;
        std::u16string playerName = SnoreSaber::Services::PlayerService::GetLocalPlayerName();

        SafePtr<ResultsViewReplayButtonController> self(this);
        SnoreSaber::Utils::Async::Run([self, replayLoader, serializedReplay = std::move(serializedReplay), beatmapLevel, beatmapKey, modifiers = std::move(modifiers), playerName] {
            try {
                replayLoader->Load(serializedReplay, beatmapLevel.ptr(), beatmapKey, modifiers, playerName);
            } catch (const std::exception& e) {
                ERROR("Failed to start replay: {}", e.what());
                SnoreSaber::Utils::Async::Main([self] {
                    if (self->watchReplayButton)
                    {
                        self->watchReplayButton->interactable = true;
                    }
                });
            }
        });
    }

    void ResultsViewReplayButtonController::SetReplayReady(bool ready)
    {
        _replayReady = ready;
        if (watchReplayButton)
        {
            watchReplayButton->interactable = ready;
        }
    }
}
