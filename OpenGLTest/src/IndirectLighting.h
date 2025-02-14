#pragma once
#include <array>
#include <glm.hpp>

class Shc
{
public:
    void SetShc(int channel, int term, float val);
    float GetShc(int channel, int term);
    float* GetData();
    
private:
    std::array<float, 27> m_data = {};
};

class IndirectLighting
{
public:
    static Shc CalcShc(const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground);
    static glm::vec3 RestoreLighting(Shc& shc, glm::vec3 direction);

    static std::array<float, 9> CalcBaseShc(const glm::vec3& direction);
    static glm::vec3 SampleColor(const glm::vec3& direction, const glm::vec3& sky, const glm::vec3& equator, const glm::vec3& ground);
};
