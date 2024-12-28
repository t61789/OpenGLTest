#pragma once
#include <string>
#include <unordered_map>
#include "Shader.h"

class Material : public ResourceBase
{
public:
    std::string name = "Unnamed Material";
    
    RESOURCE_ID shaderId = UNDEFINED_RESOURCE;
    
    std::unordered_map<std::string, int> intValues;
    std::unordered_map<std::string, bool> boolValues;
    std::unordered_map<std::string, float> floatValues;
    std::unordered_map<std::string, glm::mat4> mat4Values;
    std::unordered_map<std::string, RESOURCE_ID> textureValues;
    std::unordered_map<std::string, glm::vec4> vec4Values;

    void setIntValue(const std::string& paramName, int value);
    void setBoolValue(const std::string& paramName, bool value);
    void setFloatValue(const std::string& paramName, float value);
    void setMat4Value(const std::string& paramName, const glm::mat4& value);
    void setTextureValue(const std::string& paramName, RESOURCE_ID value);
    void setVector4Value(const std::string& paramName, const glm::vec4& value);

    void fillParams(const Shader* shader) const;
    void use(const Mesh* mesh) const;

    static RESOURCE_ID LoadFromFile(const std::string& path);
    static RESOURCE_ID CreateEmptyMaterial(const std::string& shaderPath);

    static int FillGlobalParams(const Shader* shader);

    static void SetGlobalIntValue(const std::string& paramName, int value);
    static void SetGlobalBoolValue(const std::string& paramName, bool value);
    static void SetGlobalFloatValue(const std::string& paramName, float value);
    static void SetGlobalMat4Value(const std::string& paramName, const glm::mat4& value);
    static void SetGlobalTextureValue(const std::string& paramName, RESOURCE_ID value);
    static void SetGlobalVector4Value(const std::string& paramName, const glm::vec4& value);

private:

    static std::unordered_map<std::string, int> s_globalIntValues;
    static std::unordered_map<std::string, bool> s_globalBoolValues;
    static std::unordered_map<std::string, float> s_globalFloatValues;
    static std::unordered_map<std::string, glm::mat4> s_globalMat4Values;
    static std::unordered_map<std::string, RESOURCE_ID> s_globalTextureValues;
    static std::unordered_map<std::string, glm::vec4> s_globalVec4Values;
};
