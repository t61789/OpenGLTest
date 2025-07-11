#pragma once

#include <iomanip>
#include <sstream>
#include <cstring>

#include "const.h"
#include "math/simd_math.h"

namespace op
{
    struct alignas(16) Vec3
    {
    public:
        float x = 0;
        float y = 0;
        float z = 0;
        
    private:
        float m_dummy = 0;

    #pragma region constructors

    public:
        Vec3() = default;
        
        Vec3(const float x, const float y, const float z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        
        Vec3(const float f)
        {
            x = f;
            y = f;
            z = f;
        }
        
        explicit Vec3(const float* data)
        {
            x = data[0];
            y = data[1];
            z = data[2];
        }

    #pragma endregion

    #pragma region operators

        Vec3 operator+(const Vec3& other) const
        {
            Vec3 result;
            add(&x, &other.x, &result.x);
            return result;
        }

        Vec3 operator-(const Vec3& other) const
        {
            Vec3 result;
            sub(&x, &other.x, &result.x);
            return result;
        }

        Vec3 operator*(const Vec3& other) const
        {
            Vec3 result;
            mul(&x, &other.x, &result.x);
            return result;
        }

        Vec3 operator/(const Vec3& other) const
        {
            Vec3 result;
            div(&x, &other.x, &result.x);
            return result;
        }
        
        bool operator==(const Vec3& other) const
        {
            return std::fabs(x - other.x) < EPSILON && std::fabs(y - other.y) < EPSILON && std::fabs(z - other.z) < EPSILON;
        }
        
        bool operator!=(const Vec3& other) const
        {
            return !(*this == other);
        }
        
        Vec3 operator-() const
        {
            return {-x, -y, -z};
        }

        Vec3 Cross(const Vec3 other) const
        {
            Vec3 result;
            cross(&x, &other.x, &result.x);
            return result;
        }

        Vec3 Normalize() const
        {
            Vec3 result;
            normalize(&x, &result.x);
            return result;
        }

        float Dot(const Vec3 other) const
        {
            float result;
            dot(&x, &other.x, &result);
            return result;
        }

        float Magnitude() const
        {
            float result;
            length(&x, &result);
            return result;
        }
        
        std::string ToString() const
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << '(' << x << ", " << y << ", " << z << ')';
            return ss.str();
        }
        
    #pragma endregion

    #pragma region presets

        static Vec3 Up()
        {
            return {0, 1, 0};
        }
        
        static Vec3 Right()
        {
            return {1, 0, 0};
        }

        static Vec3 Down()
        {
            return {0, -1, 0};
        }

        static Vec3 Forward()
        {
            return {0, 0, 1};
        }

        static Vec3 Back()
        {
            return {0, 0, -1};
        }

        static Vec3 Zero()
        {
            return {0, 0, 0};
        }

        static Vec3 One()
        {
            return {1, 1, 1};
        }

        static Vec3 Infinity()
        {
            return {INFINITY, INFINITY, INFINITY};
        }

        static Vec3 NegativeInfinity()
        {
            return {-INFINITY, -INFINITY, -INFINITY};
        }
        
    #pragma endregion 
    };

    struct alignas(16) Vec4
    {
        float x = 0;
        float y = 0;
        float z = 0;
        float w = 0;

    #pragma region constructors
        
        Vec4() = default;
        
        Vec4(const float x, const float y, const float z, const float w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        
        Vec4(const float f)
        {
            x = f;
            y = f;
            z = f;
            w = f;
        }

        Vec4(const Vec3 v3, const float w)
        {
            memcpy(&x, &v3.x, sizeof(float) * 4);
        }

        explicit Vec4(const float* data)
        {
            memcpy(this, data, sizeof(float) * 4);
        }
        
    #pragma endregion

    #pragma region operators
        
        Vec4 operator+(const Vec4& other) const
        {
            Vec4 result;
            add(&x, &other.x, &result.x);
            return result;
        }

        Vec4 operator-(const Vec4& other) const
        {
            Vec4 result;
            sub(&x, &other.x, &result.x);
            return result;
        }

        Vec4 operator*(const Vec4& other) const
        {
            Vec4 result;
            mul(&x, &other.x, &result.x);
            return result;
        }

        Vec4 operator/(const Vec4& other) const
        {
            Vec4 result;
            div(&x, &other.x, &result.x);
            return result;
        }
        
        bool operator==(const Vec4& other) const
        {
            return std::fabs(x - other.x) < EPSILON && std::fabs(y - other.y) < EPSILON && std::fabs(z - other.z) < EPSILON && std::fabs(w - other.w) < EPSILON;
        }
        
        Vec4 operator-() const
        {
            return {-x, -y, -z, -w};
        }

        std::string ToString()
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << '(' << x << ", " << y << ", " << z << ", " << w << ')';
            return ss.str();
        }
        
    #pragma endregion
    };
}
    