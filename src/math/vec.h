#pragma once

#include <iomanip>
#include <sstream>
#include <cstring>

#include <nlohmann/json.hpp>

#include "const.h"
#include "math/simd_math.h"

namespace op
{
    struct Vec4;
    struct Vec3;
    struct Vec2;
    
    struct alignas(16) Vec2
    {
    public:
        float x = 0;
        float y = 0;
        
    private:
        float m_dummy0 = 0;
        float m_dummy1 = 0;

    #pragma region constructors

    public:
        Vec2() = default;
        
        Vec2(const float x, const float y)
        {
            this->x = x;
            this->y = y;
        }
        
        Vec2(const float f)
        {
            x = f;
            y = f;
        }
        
        explicit Vec2(const float* data)
        {
            x = data[0];
            y = data[1];
        }

    #pragma endregion

    #pragma region operators

        Vec2 operator+(const Vec2& other) const
        {
            Vec2 result;
            add(&x, &other.x, &result.x);
            return result;
        }

        Vec2 operator+=(const Vec2& other)
        {
            add(&x, &other.x, &x);
            return *this;
        }

        Vec2 operator-(const Vec2& other) const
        {
            Vec2 result;
            sub(&x, &other.x, &result.x);
            return result;
        }

        Vec2 operator*(const Vec2& other) const
        {
            Vec2 result;
            mul(&x, &other.x, &result.x);
            return result;
        }

        Vec2 operator/(const Vec2& other) const
        {
            Vec2 result;
            div(&x, &other.x, &result.x);
            return result;
        }
        
        bool operator==(const Vec2& other) const
        {
            return std::fabs(x - other.x) < EPSILON && std::fabs(y - other.y) < EPSILON;
        }
        
        bool operator!=(const Vec2& other) const
        {
            return !(*this == other);
        }
        
        Vec2 operator-() const
        {
            return {-x, -y};
        }

        Vec2 Normalize() const
        {
            Vec2 result;
            normalize(&x, &result.x);
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
            ss << '(' << x << ", " << y << ')';
            return ss.str();
        }
        
    #pragma endregion
    };
    
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

        Vec3(const Vec4& v4);

    #pragma endregion

    #pragma region operators

        Vec3 operator+(const Vec3& other) const
        {
            Vec3 result;
            add(&x, &other.x, &result.x);
            return result;
        }

        Vec3 operator+=(const Vec3& other)
        {
            add(&x, &other.x, &x);
            return *this;
        }

        Vec3 operator-(const Vec3& other) const
        {
            Vec3 result;
            sub(&x, &other.x, &result.x);
            return result;
        }
        
        Vec3 operator-=(const Vec3& other)
        {
            sub(&x, &other.x, &x);
            return *this;
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

        friend Vec3 operator*(const float& f, const Vec3& v3)
        {
            return v3 * f;
        }
        
        float& operator[](const size_t i)
        {
            switch (i)
            {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                default:
                    throw std::out_of_range("Index out of range");
            }
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

        static const Vec3& Up()
        {
            static auto result = Vec3(0, 1, 0);
            return result;
        }

        static const Vec3& Down()
        {
            static auto result = Vec3(0, -1, 0);
            return result;
        }
        
        static const Vec3& Right()
        {
            static auto result = Vec3(1, 0, 0);
            return result;
        }
        
        static const Vec3& Left()
        {
            static auto result = Vec3(-1, 0, 0);
            return result;
        }
        
        static const Vec3& Forward()
        {
            static auto result = Vec3(0, 0, 1);
            return result;
        }
        
        static const Vec3& Back()
        {
            static auto result = Vec3(0, 0, -1);
            return result;
        }
        
        static const Vec3& Zero()
        {
            static auto result = Vec3(0, 0, 0);
            return result;
        }
        
        static const Vec3& One()
        {
            static auto result = Vec3(1, 1, 1);
            return result;
        }
        
        static const Vec3& Infinity()
        {
            static auto result = Vec3(INFINITY, INFINITY, INFINITY);
            return result;
        }
        
        static const Vec3& NegativeInfinity()
        {
            static auto result = Vec3(-INFINITY, -INFINITY, -INFINITY);
            return result;
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
            this->w = w;
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
        
        friend Vec4 operator*(const float& f, const Vec4& v4)
        {
            return v4 * f;
        }

        Vec4 operator/(const Vec4& other) const
        {
            Vec4 result;
            div(&x, &other.x, &result.x);
            return result;
        }
        
        void operator/=(const Vec4& other)
        {
            *this = *this / other;
        }
        
        bool operator==(const Vec4& other) const
        {
            return std::fabs(x - other.x) < EPSILON && std::fabs(y - other.y) < EPSILON && std::fabs(z - other.z) < EPSILON && std::fabs(w - other.w) < EPSILON;
        }
        
        Vec4 operator-() const
        {
            return {-x, -y, -z, -w};
        }

        Vec3 ToVec3() const
        {
            return Vec3(x, y, z);
        }

        Vec4 Normalize() const
        {
            Vec4 result;
            normalize(&x, &result.x);
            return result;
        }

        std::string ToString()
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << '(' << x << ", " << y << ", " << z << ", " << w << ')';
            return ss.str();
        }
        
    #pragma endregion

    #pragma region presets

        static const Vec4& Zero()
        {
            static auto result = Vec4(0, 0, 0, 0);
            return result;
        }
        
    #pragma endregion 
    };

    inline Vec3::Vec3(const Vec4& v4)
    {
        memcpy(&x, &v4.x, sizeof(float) * 3);
    }
    
    inline void from_json(const nlohmann::json& j, Vec3& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }
    
    inline void from_json(const nlohmann::json& j, Vec4& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }
}
