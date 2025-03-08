#pragma once
#include <array>
#include "glm/glm.hpp"

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
    glm::vec3 sky;
    glm::vec3 equator;
    glm::vec3 ground;

    bool Equals(const GradientAmbientColor& other)
    {
        return sky == other.sky && equator == other.equator && ground == other.ground;
    }
};

class IndirectLighting
{
public:
    static GradientAmbientColor s_gradientAmbientColor;

    static void SetGradientAmbientColor(const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground);
    
    static Shc CalcShc(const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground);
    static glm::vec3 RestoreLighting(Shc& shc, glm::vec3 direction);

    static std::array<float, 9> CalcBaseShc(const glm::vec3& direction);
    static glm::vec3 SampleColor(const glm::vec3& direction, const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground);
};
