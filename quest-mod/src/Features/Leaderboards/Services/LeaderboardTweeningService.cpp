#include "Features/Leaderboards/Services/LeaderboardTweeningService.hpp"

#include "logging.hpp"

#include <GlobalNamespace/EaseType.hpp>
#include <System/Action.hpp>
#include <System/Action_1.hpp>
#include <Tweening/FloatTween.hpp>
#include <UnityEngine/CanvasGroup.hpp>
#include <UnityEngine/Color.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Vector3.hpp>
#include <custom-types/shared/delegate.hpp>

#include <vector>

using namespace GlobalNamespace;
using namespace UnityEngine;

DEFINE_TYPE(SnoreSaber::Features::Leaderboards::Services, LeaderboardTweeningService);

namespace SnoreSaber::Features::Leaderboards::Services
{
    namespace
    {
        void SetImageAlpha(SafePtrUnity<HMUI::ImageView>& imageView, float alpha)
        {
            if (!imageView.isAlive())
            {
                return;
            }

            Color color = imageView->color;
            color.a = alpha;
            imageView->color = color;
        }

        CanvasGroup* GetOrAddCanvasGroup(RectTransform* transform)
        {
            auto canvasGroup = transform->GetComponent<CanvasGroup*>();
            if (!canvasGroup)
            {
                canvasGroup = transform->gameObject->AddComponent<CanvasGroup*>();
            }

            return canvasGroup;
        }

        void SetPromptValue(SafePtrUnity<RectTransform>& promptRoot, SafePtrUnity<CanvasGroup>& canvasGroup, float hiddenY, float visibleY, float value)
        {
            if (canvasGroup.isAlive())
            {
                canvasGroup->alpha = value;
            }
            if (promptRoot.isAlive())
            {
                Vector3 position = promptRoot->localPosition;
                position.y = hiddenY + (visibleY - hiddenY) * value;
                promptRoot->localPosition = position;
            }
        }
    }

    void LeaderboardTweeningService::ctor(Tweening::TimeTweeningManager* timeTweeningManager)
    {
        INVOKE_CTOR();
        _timeTweeningManager = timeTweeningManager;
    }

    void LeaderboardTweeningService::CreateImageViewFade(const std::string& id, float from, float to, float duration, HMUI::ImageView* imageView)
    {
        if (!imageView)
        {
            return;
        }

        SafePtrUnity<HMUI::ImageView> imageViewSafe(imageView);
        auto onUpdate = custom_types::MakeDelegate<System::Action_1<float>*>(classof(System::Action_1<float>*), (std::function<void(float)>)[imageViewSafe](float value) mutable {
            SetImageAlpha(imageViewSafe, value);
        });
        auto tween = Tweening::FloatTween::New_ctor(from, to, onUpdate, duration, EaseType::Linear, 0.0f);

        SafePtr<LeaderboardTweeningService> self(this);
        Tweening::Tween* tweenPtr = tween;
        std::string tweenId = id;
        auto finish = (std::function<void()>)[self, tweenId, tweenPtr, imageViewSafe, to]() mutable {
            self->ForgetTween(tweenId, tweenPtr);
            SetImageAlpha(imageViewSafe, to);
        };
        tween->onCompleted = custom_types::MakeDelegate<System::Action*>(classof(System::Action*), finish);
        tween->onKilled = custom_types::MakeDelegate<System::Action*>(classof(System::Action*), finish);
        CreateTween(id, tween, imageView->transform);
    }

    void LeaderboardTweeningService::CreatePromptTween(const std::string& id, float from, float to, float duration, float delay, RectTransform* promptRoot, float hiddenY, float visibleY,
                                                       std::function<void()> onCompleted)
    {
        if (!promptRoot)
        {
            return;
        }

        SafePtrUnity<RectTransform> promptRootSafe(promptRoot);
        SafePtrUnity<CanvasGroup> canvasGroupSafe(GetOrAddCanvasGroup(promptRoot));
        promptRoot->gameObject->SetActive(true);

        auto onUpdate = custom_types::MakeDelegate<System::Action_1<float>*>(classof(System::Action_1<float>*), (std::function<void(float)>)[promptRootSafe, canvasGroupSafe, hiddenY, visibleY](float value) mutable {
            SetPromptValue(promptRootSafe, canvasGroupSafe, hiddenY, visibleY, value);
        });
        auto tween = Tweening::FloatTween::New_ctor(from, to, onUpdate, duration, EaseType::OutCubic, delay);

        SafePtr<LeaderboardTweeningService> self(this);
        Tweening::Tween* tweenPtr = tween;
        std::string tweenId = id;
        tween->onCompleted = custom_types::MakeDelegate<System::Action*>(classof(System::Action*), (std::function<void()>)[self, tweenId, tweenPtr, promptRootSafe, canvasGroupSafe, hiddenY, visibleY, to, onCompleted]() mutable {
            self->ForgetTween(tweenId, tweenPtr);
            SetPromptValue(promptRootSafe, canvasGroupSafe, hiddenY, visibleY, to);
            if (onCompleted)
            {
                onCompleted();
            }
        });
        tween->onKilled = custom_types::MakeDelegate<System::Action*>(classof(System::Action*), (std::function<void()>)[self, tweenId, tweenPtr]() mutable {
            self->ForgetTween(tweenId, tweenPtr);
        });
        CreateTween(id, tween, promptRoot);
    }

    void LeaderboardTweeningService::KillTween(const std::string& id)
    {
        auto it = _activeTweens.find(id);
        if (it == _activeTweens.end())
        {
            return;
        }

        SafePtr<Tweening::Tween> tween = it->second;
        _activeTweens.erase(it);
        try
        {
            auto onKilled = tween->onKilled;
            tween->onKilled = nullptr;
            tween->Kill();
            if (onKilled)
            {
                onKilled->Invoke();
            }
        }
        catch (const std::exception& exception)
        {
            ERROR("Error killing tween {:s}: {:s}", id, exception.what());
        }
    }

    void LeaderboardTweeningService::ClearAllTweens()
    {
        std::vector<std::string> keysToRemove;
        keysToRemove.reserve(_activeTweens.size());
        for (const auto& [key, tween] : _activeTweens)
        {
            keysToRemove.push_back(key);
        }

        for (const auto& key : keysToRemove)
        {
            KillTween(key);
        }
    }

    void LeaderboardTweeningService::ClearTweensByPrefix(const std::string& prefix)
    {
        std::vector<std::string> keysToRemove;
        for (const auto& [key, tween] : _activeTweens)
        {
            if (key.rfind(prefix, 0) == 0)
            {
                keysToRemove.push_back(key);
            }
        }

        for (const auto& key : keysToRemove)
        {
            KillTween(key);
        }
    }

    void LeaderboardTweeningService::CreateTween(const std::string& id, Tweening::Tween* tween, UnityEngine::Object* owner)
    {
        KillTween(id);
        _activeTweens[id] = tween;
        _timeTweeningManager->AddTween(tween, owner ? static_cast<System::Object*>(owner) : reinterpret_cast<System::Object*>(this), false);
    }

    void LeaderboardTweeningService::ForgetTween(const std::string& id, Tweening::Tween* tween)
    {
        auto it = _activeTweens.find(id);
        if (it != _activeTweens.end() && it->second.ptr() == tween)
        {
            _activeTweens.erase(it);
        }
    }
}
