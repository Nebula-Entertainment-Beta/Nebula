/**
 * @file math_types.h
 * @brief Small POD math types for Nebula's **public** headers (no glm in `include/`).
 *
 * `Mat4` is column-major (same memory layout as glm and `glUniformMatrix4fv` with `GL_FALSE`).
 */
#pragma once

#include <array>
#include <cstddef>

namespace Nebula
{

struct Vec3
{
    float x{};
    float y{};
    float z{};
};

struct Vec4
{
    float x{};
    float y{};
    float z{};
    float w{};
};

struct Mat4
{
    /** Column-major: column `c`, row `r` at index `c * 4 + r`. */
    std::array<float, 16> cols{};

    const float *data() const { return cols.data(); }
};

inline Mat4 operator*(const Mat4 &a, const Mat4 &b)
{
    Mat4 r{};
    for (int c = 0; c < 4; ++c)
    {
        for (int row = 0; row < 4; ++row)
        {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                s += a.cols[static_cast<std::size_t>(k * 4 + row)] *
                     b.cols[static_cast<std::size_t>(c * 4 + k)];
            }
            r.cols[static_cast<std::size_t>(c * 4 + row)] = s;
        }
    }
    return r;
}

inline Vec3 &operator+=(Vec3 &a, const Vec3 &b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

inline Vec3 operator+(Vec3 a, const Vec3 &b)
{
    a += b;
    return a;
}

inline Vec3 operator*(Vec3 v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

inline Vec3 operator*(float s, Vec3 v)
{
    return v * s;
}

} // namespace Nebula
