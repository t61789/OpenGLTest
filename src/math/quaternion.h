#pragma once

#include "utils.h"
#include "math/const.h"
#include "math/vec.h"
#include "math/simd_math.h"

namespace op
{
    struct Quaternion
    {
    private:
        Vec4 m_data;
        Vec4 m_data_normalized;

        bool m_dirty = true;
    public:
        Quaternion() = default;

        Quaternion(const float x, const float y, const float z, const float w)
        {
            m_data = Vec4(x, y, z, w);
        }
        
        explicit Quaternion(const float* data)
        {
            m_data = Vec4(data);
        }

        Quaternion Normalize()
        {
            CalcNormalizedData();
            return Quaternion(&m_data_normalized.x);
        }
        
        Quaternion operator*(const Quaternion& other)
        {
            Quaternion result;
            QuaternionMul(&m_data.x, &other.m_data.x, &result.m_data.x);
            return result;
        }

        Vec3 operator*(const Vec3& v)
        {
            CalcNormalizedData();

            auto conjugate = Vec4(-m_data_normalized.x, -m_data_normalized.y, -m_data_normalized.z, m_data_normalized.w);
            Vec4 temp;
            Vec4 result;
            QuaternionMul(&m_data_normalized.x, &v.x, &temp.x);
            QuaternionMul(&temp.x, &conjugate.x, &result.x);
            
            return {result.x, result.y, result.z};
        }

        std::string ToString()
        {
            return m_data.ToString();
        }

        static Quaternion Euler(float x, float y, float z)
        {
            // order: Z -> X -> Y
            // v1 = qy * qx * qz * v0
            
            x *= DEG2RAD;
            y *= DEG2RAD;
            z *= DEG2RAD;
            
            auto cx = std::cos(x * 0.5f);
            auto sx = std::sin(x * 0.5f);
            auto cy = std::cos(y * 0.5f);
            auto sy = std::sin(y * 0.5f);
            auto cz = std::cos(z * 0.5f);
            auto sz = std::sin(z * 0.5f);

            auto nx = cz * sx * cy + sz * cx * sy;
            auto ny = cz * cx * sy - sz * sx * cy;
            auto nz = sz * cx * cy - cz * sx * sy;
            auto nw = cz * cx * cy + sz * sx * sy;

            return {nx, ny, nz, nw};
        }

        static Quaternion RotateAxis(const Vec3 axis, const float radius)
        {
            auto ca = std::cos(radius * 0.5f);
            auto sa = std::sin(radius * 0.5f);

            return {axis.x * sa, axis.y * sa, axis.z * sa, ca};
        }

        static Quaternion FaceDir(const Vec3 dir)
        {
            // auto dir_xz = dir;
            // dir_xz.y = 0;
            // dir_xz = dir_xz.Normalize();
            //
            // auto axis_x = vec3(), axis_y = vec3();
            // auto radius_x = 0.0f, radius_y = 0.0f;
            //
            // if (dir_xz != dir)
            // {
            //     axis_x = dir_xz.Cross(dir).Normalize();
            //     radius_x = std::acos(dir_xz.Dot(dir));
            // }
            //
            // if (dir_xz != vec3::Up())
            // { 
            //     axis_y = vec3::Up().Cross(dir_xz).Normalize();
            //     radius_y = std::acos(vec3::Forward().Dot(dir_xz));
            // }
            //
            // return RotateAxis(axis_x, radius_x) * RotateAxis(axis_y, radius_y);

            if (dir == Vec3::Forward())
            {
                return Identity();
            }

            if (dir == Vec3::Back())
            {
                return {0, 1, 0, 0};
            }

            if (dir == Vec3::Up())
            {
                return {-0.707107f, 0, 0, 0.707107f};
            }

            if (dir == Vec3::Down())
            {
                return {0.707107f, 0, 0, 0.707107f};
            }

            auto yaw = std::atan2(dir.z, dir.x) * RAD2DEG;
            auto pitch = -std::asin(dir.y) * RAD2DEG;

            return Euler(pitch, yaw, 0);
        }

        static Quaternion Identity()
        {
            return {0, 0, 0, 1};
        }

    private:
        void CalcNormalizedData()
        {
            if (!m_dirty)
            {
                return;
            }
            
            normalize_safe(&m_data.x, &m_data_normalized.x);
            m_dirty = false;
        }

        static void QuaternionMul(const float* l, const float* r, float* result)
        {
            result[0] = l[3] * r[0] + l[0] * r[3] + l[1] * r[2] - l[2] * r[1];
            result[1] = l[3] * r[1] + l[1] * r[3] + l[2] * r[0] - l[0] * r[2];
            result[2] = l[3] * r[2] + l[2] * r[3] + l[0] * r[1] - l[1] * r[0];
            result[3] = l[3] * r[3] - l[0] * r[0] - l[1] * r[1] - l[2] * r[2];
        }
    };
}
