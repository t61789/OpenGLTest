#pragma once
#include <array>

#include "math/math.h"

namespace op
{
    class Shc
    {
    public:
        void SetShc(int channel, int term, float val);
        void AddShc(int channel, int term, float val);
        float GetShc(int channel, int term);
        float* GetData();
        
    private:
        std::array<float, 27> m_data = {};
    };

    struct GradientAmbientColor
    {
        Vec3 sky;
        Vec3 equator;
        Vec3 ground;

        bool Equals(const GradientAmbientColor& other)
        {
            return sky == other.sky && equator == other.equator && ground == other.ground;
        }
    };

    class IndirectLighting
    {
    public:
        static GradientAmbientColor s_gradientAmbientColor;

        static void SetGradientAmbientColor(const Vec3& sky, const Vec3& equator, const Vec3& ground);
        
        static Shc CalcShc(const Vec3& sky, const Vec3& equator, const Vec3& ground);
        static Vec3 RestoreLighting(Shc& shc, Vec3 direction);

        static std::array<float, 9> CalcBaseShc(const Vec3& direction);
        static Vec3 SampleColor(const Vec3& direction, const Vec3& sky, const Vec3& equator, const Vec3& ground);
    };
}
