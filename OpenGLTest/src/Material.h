#pragma once
#include <string>
#include <unordered_map>

#include "CullMode.h"
#include "Shader.h"

class Material : public ResourceBase
{
public:
    ~Material();
    
    std::string name = "Unnamed Material";
    
    RESOURCE_ID shaderId = UNDEFINED_RESOURCE;

    CullMode cullMode = Back;

    std::unordered_map<std::string, int> intValues;
    std::unordered_map<std::string, bool> boolValues;
    std::unordered_map<std::string, float> floatValues;
    std::unordered_map<std::string, glm::mat4> mat4Values;
    std::unordered_map<std::string, RESOURCE_ID> textureValues;
    std::unordered_map<std::string, glm::vec4> vec4Values;
    std::unordered_map<std::string, std::vector<float>*> floatArrValues;

    void SetIntValue(const std::string& paramName, int value);
    void SetBoolValue(const std::string& paramName, bool value);
    void SetFloatValue(const std::string& paramName, float value);
    void SetMat4Value(const std::string& paramName, const glm::mat4& value);
    void SetTextureValue(const std::string& paramName, RESOURCE_ID value);
    void SetVector4Value(const std::string& paramName, const glm::vec4& value);
    void SetFloatArrValue(const std::string& paramName, const float *value, int count);

    void FillParams(const Shader* shader) const;
    void Use(const Mesh* mesh) const;

    static RESOURCE_ID LoadFromFile(const std::string& path);
    static RESOURCE_ID CreateEmptyMaterial(const std::string& shaderPath);

    static void SetGlobalIntValue(const std::string& paramName, int value);
    static void SetGlobalBoolValue(const std::string& paramName, bool value);
    static void SetGlobalFloatValue(const std::string& paramName, float value);
    static void SetGlobalMat4Value(const std::string& paramName, const glm::mat4& value);
    static void SetGlobalTextureValue(const std::string& paramName, RESOURCE_ID value);
    static void SetGlobalVector4Value(const std::string& paramName, const glm::vec4& value);
    static void SetGlobalFloatArrValue(const std::string& paramName, const float *value, int count);

private:
    static Material* s_globalMaterial;
    static Material* s_tempMaterial;
};
