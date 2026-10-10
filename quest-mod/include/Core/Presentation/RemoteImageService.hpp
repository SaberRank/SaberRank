#pragma once

#include <GlobalNamespace/ICoroutineStarter.hpp>
#include <System/IDisposable.hpp>
#include <System/Threading/CancellationToken.hpp>
#include <UnityEngine/Sprite.hpp>
#include <beatsaber-hook/shared/utils/typedefs-wrappers.hpp>
#include <custom-types/shared/coroutine.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <functional>
#include <map>
#include <queue>
#include <string>

DECLARE_CLASS_CODEGEN_INTERFACES(
        SnoreSaber::Core::Presentation,
        RemoteImageService,
        System::Object,
        System::IDisposable*) {
    DECLARE_INSTANCE_FIELD_PRIVATE(GlobalNamespace::ICoroutineStarter*, _coroutineStarter);
    DECLARE_CTOR(ctor, GlobalNamespace::ICoroutineStarter* coroutineStarter);
    DECLARE_OVERRIDE_METHOD_MATCH(void, Dispose, &::System::IDisposable::Dispose);

public:
    void LoadSprite(std::string url, std::function<void(UnityEngine::Sprite*)> onSuccess, std::function<void(std::string)> onFailure, System::Threading::CancellationToken cancellationToken);

private:
    static constexpr std::size_t MaxSpriteCacheSize = 150;

    custom_types::Helpers::Coroutine GetSprite(std::string url, std::function<void(UnityEngine::Sprite*)> onSuccess, std::function<void(std::string)> onFailure, System::Threading::CancellationToken cancellationToken);
    void AddSpriteToCache(const std::string& url, UnityEngine::Sprite* sprite);
    void MaintainSpriteCache();
    static void DestroySprite(UnityEngine::Sprite* sprite);

    std::map<std::string, SafePtrUnity<UnityEngine::Sprite>> _cachedSprites;
    std::queue<std::string> _spriteCacheQueue;
};
