#pragma once

#include <array>

#include "simd_math.h"
#include "vec.h"
#include "quaternion.h"

namespace op
{
    inline float clamp(const float x, const float min, const float max)
    {
        return x < min ? min : x > max ? max : x;
    }

    inline float saturate(const float x)
    {
        return clamp(x, 0.0f, 1.0f);
    }
    
    inline Vec3 lerp(const Vec3 a, const Vec3 b, const float t)
    {
        return a + t * (b - a);
    }

    inline Quaternion lerp(const Quaternion& a, const Quaternion& b, const float t)
    {
        return Quaternion(a.GetData() + t * (b.GetData() - a.GetData()));
    }
    
    inline float dot(Quaternion& a, Quaternion& b)
    {
        auto aData = a.GetNormalizedData();
        auto bData = b.GetNormalizedData();
        
        float result;
        dot(&aData.x, &bData.x, &result);

        return result;
    }
    
    inline float dot(const Vec4& a, const Vec4& b)
    {
        float result;
        dot(&a.x, &b.x, &result);

        return result;
    }
    
    inline float dot(const Vec3& a, const Vec3& b)
    {
        float result;
        dot(&a.x, &b.x, &result);

        return result;
    }

    inline Quaternion slerp(const Quaternion& a, const Quaternion& b, float t)
    {
        t = saturate(t);
        
        auto aa = a;
        auto bb = b;

        aa.Normalize();
        bb.Normalize();

        auto d = dot(aa, bb);
        if (d < 0.0f)
        {
            d = -d;
            bb = -bb;
        }

        if (d > 0.9995f)
        {
            return lerp(aa, bb, t);
        }

        auto theta = std::acosf(d);
        auto sinTheta = std::sinf(theta);
        auto scale0 = std::sinf((1.0f - t) * theta) / sinTheta;
        auto scale1 = std::sinf(t * theta) / sinTheta;

        return Quaternion(aa.GetData() * scale0 + bb.GetData() * scale1);
    }

    inline Vec3 abs(const Vec3& v)
    {
        return {std::abs(v.x), std::abs(v.y), std::abs(v.z)};
    }

    inline Vec3 sign(const Vec3& v)
    {
        Vec3 result;

        sign(&v.x, &result.x);

        return result;
    }

    inline Vec3 max(const Vec3& a, const Vec3& b)
    {
        Vec3 result;

        max(&a.x, &b.x, &result.x);

        return result;
    }

    inline Vec3 min(const Vec3& a, const Vec3& b)
    {
        Vec3 result;

        min(&a.x, &b.x, &result.x);

        return result;
    }
    
    inline Vec4 sign(const Vec4& v)
    {
        Vec4 result;

        sign(&v.x, &result.x);

        return result;
    }

    inline std::array<Vec4, 6> get_frustum_planes(Matrix4x4& vp)
    {
        std::array<Vec4, 6> planes; // 法线朝向视锥体内侧，表达式 Ax + By + Cz + D = 0
        planes[0] = Vec4(vp[0]) + Vec4(vp[3]);  // 左
        planes[1] = -Vec4(vp[0]) + Vec4(vp[3]); // 右
        planes[2] = Vec4(vp[1]) + Vec4(vp[3]);  // 下
        planes[3] = -Vec4(vp[1]) + Vec4(vp[3]); // 上
        planes[4] = Vec4(vp[2]);                // 近
        planes[5] = -Vec4(vp[2]) + Vec4(vp[3]); // 远

        for (auto& plane : planes)
        {
            auto l = Vec3(plane).Magnitude();
            plane /= l;
        }

        return planes;
    }

    inline bool frustum_culling(const std::array<Vec4, 6>& planes, const Vec3& center, const Vec3& extents)
    {
        auto center4 = Vec4(center, 0);
        auto extents4 = Vec4(extents, 0);
        
        bool result = true;
        for (const auto& plane : planes)
        {
            auto s = sign(plane);
            auto offset = extents4 * s;
            offset.w = 1.0f;
            // 只要有任何一个面，包围盒完全在这个面外面，则包围盒在视锥体外面
            result = result && (
                dot(plane, center4 + offset) >= 0 ||
                dot(plane, center4 - offset) >= 0);
        }

        return result;
    }
}
