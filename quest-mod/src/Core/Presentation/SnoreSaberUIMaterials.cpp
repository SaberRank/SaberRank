#include "Core/Presentation/SnoreSaberUIMaterials.hpp"

#include "assets.hpp"
#include "logging.hpp"
#include "static.hpp"

#include <UnityEngine/AssetBundle.hpp>
#include <UnityEngine/Texture.hpp>
#include <UnityEngine/Resources.hpp>
#include <beatsaber-hook/shared/utils/utils-functions.h>
#include <bsml/shared/Helpers/getters.hpp>
#include <bsml/shared/Helpers/utilities.hpp>
#include <paper2_scotland2/shared/string_convert.hpp>
#include "questui/ArrayUtil.hpp"

#include <string>

DEFINE_TYPE(SnoreSaber::Core::Presentation, SnoreSaberUIMaterials);

namespace SnoreSaber::Core::Presentation
{
    namespace
    {
        constexpr std::string_view RoundedImageMaterialName = "UINoGlowRoundEdge";
        constexpr std::string_view FurryMaterialCacheFile = "cyanisa.furry";
        constexpr const char* FurryMaterialName = "FurMat";

        std::string FurryMaterialPath()
        {
            return SnoreSaber::Static::DATA_DIR + "/" + std::string(FurryMaterialCacheFile);
        }

        bool WriteFurryMaterialBundle(std::string const& path)
        {
            std::string_view bundleData = IncludedAssets::cyanisa_furry;
            return writefile(path, bundleData);
        }
    }

    void SnoreSaberUIMaterials::ctor()
    {
        INVOKE_CTOR();
    }

    UnityEngine::Material* SnoreSaberUIMaterials::RoundedImageMaterial()
    {
        if (!_roundedImageMaterial.isAlive())
        {
            _roundedImageMaterial = QuestUI::ArrayUtil::First(UnityEngine::Resources::FindObjectsOfTypeAll<UnityEngine::Material*>(), [](UnityEngine::Material* material) {
                return Paper::StringConvert::from_utf16(material->name) == RoundedImageMaterialName;
            });
        }

        if (_roundedImageMaterial.isAlive())
        {
            return _roundedImageMaterial.ptr();
        }

        return BSML::Helpers::GetUINoGlowMat();
    }

    UnityEngine::Material* SnoreSaberUIMaterials::DefaultFontMaterial()
    {
        if (!_defaultFontMaterial.isAlive())
        {
            _defaultFontMaterial = BSML::Helpers::GetMainUIFontMaterial();
        }

        return _defaultFontMaterial.ptr();
    }

    UnityEngine::Material* SnoreSaberUIMaterials::FurryFontMaterial()
    {
        if (_furryFontMaterial.isAlive())
        {
            return _furryFontMaterial.ptr();
        }

        auto bundlePath = FurryMaterialPath();
        if (!WriteFurryMaterialBundle(bundlePath))
        {
            ERROR("Failed to write furry font material bundle to {}", bundlePath);
            return DefaultFontMaterial();
        }

        auto bundle = UnityEngine::AssetBundle::LoadFromFile(bundlePath);
        if (!bundle)
        {
            ERROR("Failed to load furry font material bundle from {}", bundlePath);
            return DefaultFontMaterial();
        }

        auto sourceMaterial = bundle->LoadAsset<UnityEngine::Material*>(FurryMaterialName);
        if (sourceMaterial)
        {
            _furryFontMaterial = UnityEngine::Material::New_ctor(sourceMaterial);
            if (auto defaultFontMaterial = DefaultFontMaterial())
            {
                _furryFontMaterial->set_mainTexture(defaultFontMaterial->get_mainTexture());
            }
        }
        else
        {
            ERROR("Failed to load {} from furry font material bundle", FurryMaterialName);
        }

        bundle->Unload(false);
        return _furryFontMaterial.isAlive() ? _furryFontMaterial.ptr() : DefaultFontMaterial();
    }

    UnityEngine::Sprite* SnoreSaberUIMaterials::BlankSprite()
    {
        if (!_blankSprite.isAlive())
        {
            _blankSprite = BSML::Utilities::ImageResources::GetBlankSprite();
        }

        return _blankSprite.ptr();
    }

    void SnoreSaberUIMaterials::Reset()
    {
        _roundedImageMaterial = nullptr;
        _defaultFontMaterial = nullptr;
        _furryFontMaterial = nullptr;
        _blankSprite = nullptr;
    }
}
