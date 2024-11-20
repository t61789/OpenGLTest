#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Texture.h"
#include "Shader.h"

class Material
{
public:
    std::unordered_map<std::string, int> intValues;
    std::unordered_map<std::string, bool> boolValues;
    std::unordered_map<std::string, float> floatValues;
    std::unordered_map<std::string, glm::mat4> mat4Values;
    std::unordered_map<std::string, TEXTURE_ID> textureValues;

    void SetIntValue(const std::string& name, int value);
    void SetBoolValue(const std::string& name, bool value);
    void SetFloatValue(const std::string& name, float value);
    void SetMat4Value(const std::string& name, const glm::mat4& value);
    void SetTextureValue(const std::string& name, TEXTURE_ID value);

    void FillParams(const Shader* shader) const;

    static Material* LoadFromFile(const std::string& path);
};
