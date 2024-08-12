#include "Material.h"

void Material::SetIntValue(const std::string& name, const int value)
{
    intValues[name] = value;
}

void Material::SetBoolValue(const std::string& name, const bool value)
{
    boolValues[name] = value;
}

void Material::SetFloatValue(const std::string& name, const float value)
{
    floatValues[name] = value;
}

void Material::SetMat4Value(const std::string& name, const glm::mat4& value)
{
    mat4Values[name] = value;
}

void Material::SetTextureValue(const std::string& name, Texture* value)
{
    textureValues[name] = value;
}

void Material::FillParams(const Shader* shader) const
{
    for (const auto& element : intValues)
    {
        shader->setInt(element.first, element.second);
    }
    
    for (const auto& element : boolValues)
    {
        shader->setBool(element.first, element.second);
    }
    
    for (const auto& element : floatValues)
    {
        shader->setFloat(element.first, element.second);
    }
    
    for (const auto& element : mat4Values)
    {
        shader->setMatrix(element.first, element.second);
    }

    int slot = 0;
    for (auto& element : textureValues)
    {
        if(!shader->hasParam(element.first))
        {
            return;
        }
        
        shader->setTexture(element.first, slot, element.second);
        slot++;
    }
}
