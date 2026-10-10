#pragma once

namespace UnityEngine
{
    struct Quaternion
    {
        Quaternion() = default;
        constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 1.0f;
    };
} // namespace UnityEngine
