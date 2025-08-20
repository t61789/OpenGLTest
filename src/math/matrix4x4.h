#pragma once

#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>

#include "math/quaternion.h"
#include "simd_math.h"
#include "vec.h"

namespace op
{
    struct alignas(16) Matrix4x4
    {
    private:
        float m_data[16];

    public:
        Matrix4x4()
        {
            std::memset(m_data, 0, sizeof(m_data));
        }

        Matrix4x4(
            const float m00, const float m01, const float m02, const float m03,
            const float m10, const float m11, const float m12, const float m13,
            const float m20, const float m21, const float m22, const float m23,
            const float m30, const float m31, const float m32, const float m33)
        {
            m_data[0] = m00;
            m_data[1] = m01;
            m_data[2] = m02;
            m_data[3] = m03;
            m_data[4] = m10;
            m_data[5] = m11;
            m_data[6] = m12;
            m_data[7] = m13;
            m_data[8] = m20;
            m_data[9] = m21;
            m_data[10] = m22;
            m_data[11] = m23;
            m_data[12] = m30;
            m_data[13] = m31;
            m_data[14] = m32;
            m_data[15] = m33;
        }

        Matrix4x4(const Vec4 row0, const Vec4 row1, const Vec4 row2, const Vec4 row3)
        {
            memcpy(m_data, &row0.x, sizeof(float) * 4);
            memcpy(m_data + 4, &row1.x, sizeof(float) * 4);
            memcpy(m_data + 8, &row2.x, sizeof(float) * 4);
            memcpy(m_data + 12, &row3.x, sizeof(float) * 4);
        }

        explicit Matrix4x4(const float* f)
        {
            memcpy(m_data, f, sizeof(float) * 16);
        }
        
        float* operator[](const size_t i)
        {
            return m_data + i * 4;
        }

        Matrix4x4 operator*(const Matrix4x4& other) const
        {
            Matrix4x4 result;
            Matrix4x4 other_transpose = other.Transpose();
            
            for (size_t i = 0; i < 4; ++i)
            {
                __m128 l = _mm_load_ps(m_data + i * 4);
                for (size_t j = 0; j < 4; ++j)
                {
                    __m128 r = _mm_load_ps(other_transpose.m_data + j * 4);
                    r = _mm_mul_ps(l, r);
                    r = _mm_hadd_ps(r, r);
                    r = _mm_hadd_ps(r, r);

                    result[i][j] = _mm_cvtss_f32(r);
                }
            }

            return result;
        }

        Vec4 operator*(const Vec4& other) const
        {
            float result[4];
            for (size_t i = 0; i < 4; ++i)
            {
                __m128 l = _mm_load_ps(m_data + i * 4);
                __m128 r = _mm_load_ps(&other.x);
                r = _mm_mul_ps(l, r);
                r = _mm_hadd_ps(r, r);
                r = _mm_hadd_ps(r, r);
                
                result[i] = _mm_cvtss_f32(r);
            }

            return Vec4(result);
        }

        bool RudeCmp(const Matrix4x4& other) const
        {
            return memcmp(m_data, other.m_data, sizeof(float) * 16) == 0;
        }

        [[nodiscard]] Matrix4x4 Transpose() const
        {
            return {
                m_data[0], m_data[4], m_data[8], m_data[12],
                m_data[1], m_data[5], m_data[9], m_data[13],
                m_data[2], m_data[6], m_data[10], m_data[14],
                m_data[3], m_data[7], m_data[11], m_data[15]
            };
        }

        Matrix4x4 Inverse() const
        {
            // 高斯-若尔当法
            
            // 增广矩阵
            float aug[32] = {};
            for (size_t i = 0; i < 4; ++i)
            {
                memcpy(aug + i * 8, m_data + i * 4, sizeof(float) * 4);
                aug[i * 8 + 4 + i] = 1.0f;
            }

            for (size_t col = 0; col < 4; ++col)
            {
                // 寻找主元行
                auto max_row = col;
                for (size_t row = col + 1; row < 4; ++row)
                {
                    if (std::abs(aug[row * 8 + col]) > std::abs(aug[max_row * 8 + col]))
                    {
                        max_row = row;
                    }
                }

                if (std::abs(aug[max_row * 8 + col]) < EPSILON)
                {
                    throw std::runtime_error("奇异矩阵无法求逆");
                }

                // 交换当前行和主元行
                if (max_row != col)
                {
                    float temp[8];
                    memcpy(temp, aug + max_row * 8, sizeof(float) * 8);
                    memcpy(aug + max_row * 8, aug + col * 8, sizeof(float) * 8);
                    memcpy(aug + col * 8, temp, sizeof(float) * 8);
                }

                // 将主元行归一化
                auto pe = aug[col * 8 + col];
                for (size_t i = col; i < 8; ++i)
                {
                    aug[col * 8 + i] /= pe;
                }

                // 遍历每一行，利用主元行把当前行减成0
                for (size_t row = 0; row < 4; ++row)
                {
                    if (row == col)
                    {
                        continue;
                    }

                    auto factor = aug[row * 8 + col];
                    for (size_t i = col; i < 8; ++i)
                    {
                        aug[row * 8 + i] -= factor * aug[col * 8 + i];
                    }
                }
            }

            auto result = Matrix4x4();
            for (size_t i = 0; i < 4; ++i)
            {
                memcpy(result.m_data + i * 4, aug + i * 8 + 4, sizeof(float) * 4);
            }

            return result;
        }

        float* GetData()
        {
            return m_data;
        }
        
        const float* GetReadOnlyData() const
        {
            return m_data;
        }

        void SetRow(const int row, const Vec4 v)
        {
            memcpy(m_data + static_cast<size_t>(row) * 4, &v.x, sizeof(float) * 4);
        }

        Vec4 GetRow(const int row)
        {
            return Vec4(m_data + static_cast<size_t>(row) * 4);
        }

        Vec3 Forward() const
        {
            return Vec3(m_data[2], m_data[6], m_data[10]).Normalize();
        }

        Vec3 Up() const
        {
            return Vec3(m_data[1], m_data[5], m_data[9]).Normalize();
        }

        Vec3 Right() const
        {
            return Vec3(m_data[0], m_data[4], m_data[8]).Normalize();
        }

        std::string ToString() const
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << '(';

            for (int i = 0; i < 16; i++)
            {
                if (i != 0)
                {
                    ss << ", ";
                }
                ss << m_data[i];

                if (i % 4 == 3 && i != 15)
                {
                    ss << "\n";
                }
            }

            ss << ')';

            return ss.str();
        }

        static Matrix4x4 FaceDir(Vec3 forward, Vec3 up = Vec3(0, 1, 0))
        {
            if (forward == up || forward == -up)
            {
                if (up == Vec3(0, 1, 0))
                {
                    up = Vec3(1, 0, 0);
                }
                else
                {
                    up = Vec3(0, 1, 0);
                }
            }
            
            auto right = up.Cross(forward).Normalize();
            auto new_up = forward.Cross(right);

            return Matrix4x4(
                Vec4(right, 0),
                Vec4(new_up, 0),
                Vec4(forward.Normalize(), 0),
                Vec4(0, 0, 0, 1)
            ).Transpose();
        }
        
        static Matrix4x4 TRS(const Vec3& trans, const Quaternion& rot, const Vec3& scale)
        {
            auto m = Identity();
            m[0][3] = trans.x;
            m[1][3] = trans.y;
            m[2][3] = trans.z;

            auto qq = rot;
            auto q = qq.GetNormalizedData();
            
            auto x2 = q.x * q.x;
            auto xy = q.x * q.y;
            auto xz = q.x * q.z;
            auto xw = q.x * q.w;
            auto y2 = q.y * q.y;
            auto yz = q.y * q.z;
            auto yw = q.y * q.w;
            auto z2 = q.z * q.z;
            auto zw = q.z * q.w;
            
            m[0][0] = (1 - 2 * (y2 + z2)) * scale.x;
            m[0][1] = 2 * (xy - zw) * scale.y;
            m[0][2] = 2 * (xz + yw) * scale.z;
            m[1][0] = 2 * (xy + zw) * scale.x;
            m[1][1] = (1 - 2 * (x2 + z2)) * scale.y;
            m[1][2] = 2 * (yz - xw) * scale.z;
            m[2][0] = 2 * (xz - yw) * scale.x;
            m[2][1] = 2 * (yz + xw) * scale.y;
            m[2][2] = (1 - 2 * (x2 + y2)) * scale.z;

            return m;
        }

        static const Matrix4x4& Identity()
        {
            static Matrix4x4 m = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };

            return m;
        }
    };
}
