#pragma once

// host stub: plain rgba carrier matching the cordl value-type field layout
namespace UnityEngine
{
    struct Color
    {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;

        Color() = default;
        Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
    };
} // namespace UnityEngine
