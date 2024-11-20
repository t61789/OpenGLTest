#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Texture.h"
#include "Shader.h"

class Material : public ResourceBase
{
public:
    RESOURCE_ID m_shader = UNDEFINED_RESOURCE;
    
    std::unordered_map<std::string, int> m_intValues;
    std::unordered_map<std::string, bool> m_boolValues;
    std::unordered_map<std::string, float> m_floatValues;
    std::unordered_map<std::string, glm::mat4> m_mat4Values;
    std::unordered_map<std::string, RESOURCE_ID> m_textureValues;
    std::unordered_map<std::string, glm::vec4> m_vec4Values;

    void SetIntValue(const std::string& name, int value);
    void SetBoolValue(const std::string& name, bool value);
    void SetFloatValue(const std::string& name, float value);
    void SetMat4Value(const std::string& name, const glm::mat4& value);
    void SetTextureValue(const std::string& name, RESOURCE_ID value);
    void SetVector4Value(const std::string& name, const glm::vec4& value);

    void FillParams(const Shader* shader) const;

    static RESOURCE_ID LoadFromFile(const std::string& path);
};
