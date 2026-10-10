#pragma once

#include <HMUI/ImageView.hpp>
#include <Tweening/TimeTweeningManager.hpp>
#include <Tweening/Tween.hpp>
#include <UnityEngine/RectTransform.hpp>
#include <beatsaber-hook/shared/utils/typedefs-wrappers.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>
#include <functional>
#include <string>
#include <unordered_map>

DECLARE_CLASS_CODEGEN(SnoreSaber::Features::Leaderboards::Services, LeaderboardTweeningService, Il2CppObject) {
    DECLARE_INSTANCE_FIELD_PRIVATE(UnityW<Tweening::TimeTweeningManager>, _timeTweeningManager);
    DECLARE_CTOR(ctor, Tweening::TimeTweeningManager* timeTweeningManager);

  public:
    void CreateImageViewFade(const std::string& id, float from, float to, float duration, HMUI::ImageView* imageView);
    void CreatePromptTween(const std::string& id, float from, float to, float duration, float delay, UnityEngine::RectTransform* promptRoot, float hiddenY, float visibleY,
                           std::function<void()> onCompleted = nullptr);
    void KillTween(const std::string& id);
    void ClearAllTweens();
    void ClearTweensByPrefix(const std::string& prefix);

  private:
    void CreateTween(const std::string& id, Tweening::Tween* tween, UnityEngine::Object* owner);
    void ForgetTween(const std::string& id, Tweening::Tween* tween);
    std::unordered_map<std::string, SafePtr<Tweening::Tween>> _activeTweens;
};
