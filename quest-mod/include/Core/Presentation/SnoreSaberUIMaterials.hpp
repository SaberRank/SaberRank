#pragma once

#include <UnityEngine/Material.hpp>
#include <UnityEngine/Sprite.hpp>
#include <System/zzzz__Object_def.hpp>
#include <beatsaber-hook/shared/utils/typedefs-wrappers.hpp>
#include <custom-types/shared/macros.hpp>
#include <lapiz/shared/macros.hpp>

#include <string_view>

DECLARE_CLASS_CODEGEN(SnoreSaber::Core::Presentation, SnoreSaberUIMaterials, System::Object) {
    DECLARE_CTOR(ctor);

  public:
    UnityEngine::Material* RoundedImageMaterial();
    UnityEngine::Material* DefaultFontMaterial();
    UnityEngine::Material* FurryFontMaterial();
    UnityEngine::Sprite* BlankSprite();
    void Reset();

  private:
    SafePtrUnity<UnityEngine::Material> _roundedImageMaterial;
    SafePtrUnity<UnityEngine::Material> _defaultFontMaterial;
    SafePtrUnity<UnityEngine::Material> _furryFontMaterial;
    SafePtrUnity<UnityEngine::Sprite> _blankSprite;
};
