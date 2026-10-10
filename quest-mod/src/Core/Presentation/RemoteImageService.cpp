#include "Core/Presentation/RemoteImageService.hpp"

#include <System/Collections/IEnumerator.hpp>
#include <System/String.hpp>
#include <UnityEngine/GameObject.hpp>
#include <UnityEngine/Networking/DownloadHandlerTexture.hpp>
#include <UnityEngine/Networking/UnityWebRequest.hpp>
#include <UnityEngine/Networking/UnityWebRequestTexture.hpp>
#include <UnityEngine/Object.hpp>
#include <UnityEngine/Texture2D.hpp>
#include <bsml/shared/Helpers/utilities.hpp>

#include <utility>
#include <vector>

DEFINE_TYPE(SnoreSaber::Core::Presentation, RemoteImageService);

using namespace UnityEngine;
using namespace UnityEngine::Networking;

namespace SnoreSaber::Core::Presentation
{
    void RemoteImageService::ctor(GlobalNamespace::ICoroutineStarter* coroutineStarter)
    {
        INVOKE_CTOR();
        _coroutineStarter = coroutineStarter;
    }

    void RemoteImageService::LoadSprite(std::string url, std::function<void(Sprite*)> onSuccess, std::function<void(std::string)> onFailure, System::Threading::CancellationToken cancellationToken)
    {
        if (cancellationToken.IsCancellationRequested)
        {
            if (onFailure)
            {
                onFailure("Cancelled");
            }
            MaintainSpriteCache();
            return;
        }

        auto cached = _cachedSprites.find(url);
        if (cached != _cachedSprites.end() && cached->second.isAlive())
        {
            if (onSuccess)
            {
                onSuccess(cached->second.ptr());
            }
            MaintainSpriteCache();
            return;
        }

        if (!_coroutineStarter)
        {
            if (onFailure)
            {
                onFailure("Coroutine starter unavailable");
            }
            MaintainSpriteCache();
            return;
        }

        _coroutineStarter->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(GetSprite(std::move(url), std::move(onSuccess), std::move(onFailure), cancellationToken)));
        MaintainSpriteCache();
    }

    custom_types::Helpers::Coroutine RemoteImageService::GetSprite(std::string url, std::function<void(Sprite*)> onSuccess, std::function<void(std::string)> onFailure, System::Threading::CancellationToken cancellationToken)
    {
        UnityWebRequest* www = UnityWebRequestTexture::GetTexture(url);
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(www->SendWebRequest());
        auto handler = il2cpp_utils::cast<DownloadHandlerTexture>(www->downloadHandler);

        while (!www->isDone)
        {
            if (cancellationToken.IsCancellationRequested)
            {
                www->Abort();
                www->Dispose();
                if (onFailure)
                {
                    onFailure("Cancelled");
                }
                co_return;
            }
            co_yield nullptr;
        }

        if (cancellationToken.IsCancellationRequested)
        {
            www->Dispose();
            if (onFailure)
            {
                onFailure("Cancelled");
            }
            co_return;
        }

        if (www->result == UnityWebRequest::Result::ProtocolError || www->result == UnityWebRequest::Result::ConnectionError)
        {
            if (onFailure)
            {
                onFailure(www->error);
            }
            www->Dispose();
            co_return;
        }
        if (!System::String::IsNullOrEmpty(www->error))
        {
            if (onFailure)
            {
                onFailure(www->error);
            }
            www->Dispose();
            co_return;
        }
        if (!handler || !handler->texture)
        {
            if (onFailure)
            {
                onFailure("Missing texture");
            }
            www->Dispose();
            co_return;
        }

        Sprite* sprite = BSML::Utilities::LoadSpriteFromTexture(handler->texture);
        www->Dispose();
        AddSpriteToCache(url, sprite);
        if (onSuccess)
        {
            onSuccess(sprite);
        }
        co_return;
    }

    void RemoteImageService::MaintainSpriteCache()
    {
        while (_cachedSprites.size() > MaxSpriteCacheSize && !_spriteCacheQueue.empty())
        {
            std::string oldestUrl = std::move(_spriteCacheQueue.front());
            _spriteCacheQueue.pop();
            auto cached = _cachedSprites.find(oldestUrl);
            if (cached != _cachedSprites.end())
            {
                Sprite* sprite = cached->second.isAlive() ? cached->second.ptr() : nullptr;
                _cachedSprites.erase(cached);
                DestroySprite(sprite);
            }
        }

        // somehow the objects can be GCed, even when behind a SafePtrUnity
        std::vector<std::string> badSprites;
        for (auto& [key, value] : _cachedSprites)
        {
            if (!value.isAlive())
            {
                badSprites.push_back(key);
            }
        }
        for (auto& key : badSprites)
        {
            _cachedSprites.erase(key);
        }
    }

    void RemoteImageService::AddSpriteToCache(const std::string& url, Sprite* sprite)
    {
        if (_cachedSprites.contains(url))
        {
            DestroySprite(sprite);
            return;
        }

        _cachedSprites.emplace(url, sprite);
        _spriteCacheQueue.push(url);
        MaintainSpriteCache();
    }

    void RemoteImageService::Dispose()
    {
        for (auto& [key, value] : _cachedSprites)
        {
            if (value.isAlive())
            {
                DestroySprite(value.ptr());
            }
        }

        _cachedSprites.clear();
        _spriteCacheQueue = std::queue<std::string>();
    }

    void RemoteImageService::DestroySprite(Sprite* sprite)
    {
        if (!sprite)
        {
            return;
        }

        Texture2D* texture = sprite->texture;
        UnityEngine::Object::Destroy(sprite);
        if (texture)
        {
            UnityEngine::Object::Destroy(texture);
        }
    }
}
