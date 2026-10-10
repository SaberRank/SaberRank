#pragma once

namespace UnityEngine
{
    struct Vector3
    {
        Vector3() = default;
        constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };
} // namespace UnityEngine
