﻿#include "indirect_lighting.h"

#include "material.h"
#include "utils.h"

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

    void IndirectLighting::SetGradientAmbientColor(const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground)
    {
        auto cur = GradientAmbientColor{ sky, equator, ground };
        if (s_gradientAmbientColor.Equals(cur))
        {
            return;
        }

        s_gradientAmbientColor = cur;
        auto shc = CalcShc(sky, equator, ground);
        Material::SetGlobalFloatArrValue("_Shc", shc.GetData(), 27);
    }

    Shc IndirectLighting::CalcShc(const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground)
    {
        static glm::vec3 directions[6] =
        {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f)
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

    glm::vec3 IndirectLighting::RestoreLighting(Shc& shc, glm::vec3 direction)
    {
        direction = glm::normalize(direction);

        auto baseShc = CalcBaseShc(direction);
        
        glm::vec3 result;
        for (int term = 0; term < 9; ++term)
        {
            for (int channel = 0; channel < 3; ++channel)
            {
                result[channel] += baseShc[term] * shc.GetShc(channel, term);
            }
        }

        return result;
    }

    std::array<float, 9> IndirectLighting::CalcBaseShc(const glm::vec3& direction)
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

    glm::vec3 IndirectLighting::SampleColor(const glm::vec3& direction, const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground)
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
