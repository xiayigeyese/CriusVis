#pragma once

#include <memory>

#include <agz/utility/math.h>
#include <agz/utility/mesh.h>

// math

using Vec2 = agz::math::vec2f;
using Vec3 = agz::math::float3;
using Vec4 = agz::math::float4;
using Mat4 = agz::math::float4x4;
using Vec3i = agz::math::vec3i;

struct AABB { Vec3 lower, upper; };

// smart pointer

template<typename T>
using RC = std::shared_ptr<T>;

template<typename T, typename...Args>
RC<T> newRC(Args &&...args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
RC<T> toRC(T &&data)
{
    return newRC<T>(std::forward<T>(data));
}

template<typename T>
using Box = std::unique_ptr<T>;

template<typename T, typename...Args>
Box<T> newBox(Args &&...args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// unreachable hint

[[noreturn]] inline void unreachable()
{
#if defined(_MSC_VER)
    __assume(0);
#elif defined(__GNUC__)
    __builtin_unreachable();
#endif
    std::terminate();
}
