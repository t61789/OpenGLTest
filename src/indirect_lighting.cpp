#include "indirect_lighting.h"


#include "game_resource.h"
#include "material.h"
#include "utils.h"
#include "render/gl/gl_cbuffer.h"

namespace op
{
    GradientAmbientColor IndirectLighting::s_gradientAmbientColor;

    void Shc::SetShc(int channel, int term, float val)
    {
        m_data[term * 3 + channel] = val;
    }

    void Shc::AddShc(int channel, int term, float val)
    {
        m_data[term * 3 + channel] += val;
    }

    float Shc::GetShc(int channel, int term)
    {
        return m_data[term * 3 + channel];
    }

    float* Shc::GetData()
    {
        return m_data.data();
    }

    void IndirectLighting::SetGradientAmbientColor(const Vec3& sky, const Vec3& equator, const Vec3& ground)
    {
        auto cur = GradientAmbientColor{ sky, equator, ground };
        if (s_gradientAmbientColor.Equals(cur))
        {
            return;
        }

        s_gradientAmbientColor = cur;
        auto shc = CalcShc(sky, equator, ground);
        GetGlobalCbuffer()->Set(SHC, shc.GetData(), 27); // TODO
    }

    Shc IndirectLighting::CalcShc(const Vec3& sky, const Vec3& equator, const Vec3& ground)
    {
        static Vec3 directions[6] =
        {
            Vec3(1.0f, 0.0f, 0.0f),
            Vec3(-1.0f, 0.0f, 0.0f),
            Vec3(0.0f, 1.0f, 0.0f),
            Vec3(0.0f, -1.0f, 0.0f),
            Vec3(0.0f, 0.0f, 1.0f),
            Vec3(0.0f, 0.0f, -1.0f)
        };

        Shc result;
        for (auto direction : directions)
        {
            auto baseShc = CalcBaseShc(direction);
            auto color = SampleColor(direction, sky, equator, ground);
            
            for (int term = 0; term < 9; ++term)
            {
                for (int channel = 0; channel < 3; ++channel)
                {
                    result.AddShc(channel, term, baseShc[term] * color[channel]);
                }
            }
        }

        return result;
    }

    Vec3 IndirectLighting::RestoreLighting(Shc& shc, Vec3 direction)
    {
        direction = direction.Normalize();

        auto baseShc = CalcBaseShc(direction);
        
        Vec3 result;
        for (int term = 0; term < 9; ++term)
        {
            for (int channel = 0; channel < 3; ++channel)
            {
                result[channel] += baseShc[term] * shc.GetShc(channel, term);
            }
        }

        return result;
    }

    std::array<float, 9> IndirectLighting::CalcBaseShc(const Vec3& direction)
    {
        return {
            0.282095f,
            0.488603f * direction.y,
            0.488603f * direction.z,
            0.488603f * direction.x,
            1.092548f * direction.y * direction.x,
            1.092548f * direction.y * direction.z,
            0.315392f * (-direction.x * direction.x - direction.y * direction.y + 2 * direction.z * direction.z),
            1.092548f * direction.z * direction.x,
            0.546274f * (direction.x * direction.x - direction.y * direction.y)
        };
    }

    Vec3 IndirectLighting::SampleColor(const Vec3& direction, const Vec3& sky, const Vec3& equator, const Vec3& ground)
    {
        if (direction.y > 0.5f)
        {
            return sky;
        }
        
        if (direction.y < -0.5f)
        {
            return ground;
        }
        
        return equator;
    }
}
