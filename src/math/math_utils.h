#pragma once

#include <array>

#include "simd_math.h"
#include "vec.h"
#include "matrix4x4.h"
#include "quaternion.h"

namespace op
{
    inline uint32_t ceil_div(const uint32_t a, const uint32_t b)
    {
        return (a + b - 1) / b;
    }
    
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
            auto o0 = center4 + offset;
            auto o1 = center4 - offset;
            o0.w = o1.w = 1.0f;
            // 只要有任何一个面，包围盒完全在这个面外面，则包围盒在视锥体外面
            result = result && (
                dot(plane, o0) >= 0 ||
                dot(plane, o1) >= 0);
        }

        return result;
    }
    
    static Matrix4x4 create_projection(const float fov, const float aspect, const float near, const float far)
    {
        auto f = 1 / std::tan(fov * 0.5f * DEG2RAD);
        auto a = (far) / (near - far);
        auto b = (far * near) / (near - far);

        return {
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, a, b,
            0, 0, -1, 0
        };
    }
    
    static Matrix4x4 create_ortho_projection(const float r, const float l, const float t, const float b, const float f, const float n)
    {
        return {
            2 / (r - l), 0, 0, (l + r) / (l - r),
            0, 2 / (t - b), 0, (t + b) / (b - t),
            0, 0, 1 / (f - n), n / (n - f),
            0, 0, 0, 1
        };
    }

    // 将世界坐标转换为屏幕坐标的工具函数
    static Vec3 world_to_screen(const Vec3& worldPos, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, const Vec2& screenSize)
    {
        Vec4 clipSpacePos = projMatrix * viewMatrix * Vec4(worldPos, 1.0f);

        // 透视除法，转到 NDC（Normalized Device Coordinates）空间
        if (clipSpacePos.w != 0.0f) {
            clipSpacePos.x /= clipSpacePos.w;
            clipSpacePos.y /= clipSpacePos.w;
        }

        // 转换到屏幕坐标
        Vec3 screenPos;
        screenPos.x = (clipSpacePos.x * 0.5f + 0.5f) * screenSize.x; // NDC x [-1,1] -> screen x [0,width]
        screenPos.y = (1.0f - (clipSpacePos.y * 0.5f + 0.5f)) * screenSize.y; // NDC y [-1,1] -> screen y [height,0]
        screenPos.z = clipSpacePos.z;
        return screenPos;
    }
    
    // 计算点的区域码
    static int compute_region_code(float x, float y, float xmin, float ymin, float xmax, float ymax)
    {
        constexpr int inside = 0; // 0000
        constexpr int left = 1;   // 0001
        constexpr int right = 2;  // 0010
        constexpr int bottom = 4; // 0100
        constexpr int top = 8;    // 1000
        
        int code = inside;

        if (x < xmin) code |= left;    // 左侧
        else if (x > xmax) code |= right; // 右侧
        if (y < ymin) code |= bottom; // 下侧
        else if (y > ymax) code |= top;    // 上侧

        return code;
    }

    // Cohen-Sutherland线段裁剪算法
    static bool cohen_sutherland_clip(float &x1, float &y1, float &x2, float &y2, 
                             float xmin, float ymin, float xmax, float ymax)
    {
        constexpr int left = 1;   // 0001
        constexpr int right = 2;  // 0010
        constexpr int bottom = 4; // 0100
        constexpr int top = 8;    // 1000
        
        int code1 = compute_region_code(x1, y1, xmin, ymin, xmax, ymax);
        int code2 = compute_region_code(x2, y2, xmin, ymin, xmax, ymax);

        bool accept = false;

        while (true) {
            if (code1 == 0 && code2 == 0)
            {
                // 两个点都在矩形内
                accept = true;
                break;
            }
            
            if (code1 & code2)
            {
                // 两个点在矩形外同一侧
                break;
            }
            
            // 线段至少部分在矩形内
            int codeOut;
            float x = 0, y = 0;

            // 选择一个在矩形外的点
            if (code1 != 0)
            {
                codeOut = code1;
            }
            else
            {
                codeOut = code2;
            }

            // 找到交点
            if (codeOut & top)
            { // 上侧
                x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
                y = ymax;
            }
            else if (codeOut & bottom)
            { // 下侧
                x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
                y = ymin;
            }
            else if (codeOut & right)
            { // 右侧
                y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
                x = xmax;
            }
            else if (codeOut & left)
            { // 左侧
                y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
                x = xmin;
            }

            // 更新点的位置，并重新计算区域码
            if (codeOut == code1)
            {
                x1 = x;
                y1 = y;
                code1 = compute_region_code(x1, y1, xmin, ymin, xmax, ymax);
            }
            else
            {
                x2 = x;
                y2 = y;
                code2 = compute_region_code(x2, y2, xmin, ymin, xmax, ymax);
            }
        }

        return accept;
    }
}
