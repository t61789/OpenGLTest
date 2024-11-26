#include "Material.h"

#include <fstream>

#include "Utils.h"
#include "../lib/json.hpp"

void Material::setIntValue(const std::string& name, const int value)
{
    intValues[name] = value;
}

void Material::setBoolValue(const std::string& name, const bool value)
{
    boolValues[name] = value;
}

void Material::setFloatValue(const std::string& name, const float value)
{
    floatValues[name] = value;
}

void Material::setMat4Value(const std::string& name, const glm::mat4& value)
{
    mat4Values[name] = value;
}

void Material::setTextureValue(const std::string& name, const RESOURCE_ID value)
{
    textureValues[name] = value;
}

void Material::setVector4Value(const std::string& name, const glm::vec4& value)
{
    vec4Values[name] = value;
}

void Material::fillParams(const Shader* shader) const
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
    
    for (const auto& element : vec4Values)
    {
        shader->setVector(element.first, element.second);
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

RESOURCE_ID Material::LoadFromFile(const std::string& path)
{
    if(ResourceMgr::IsResourceRegistered(path))
    {
        return ResourceMgr::GetRegisteredResource(path);
    }
    
    auto s = std::ifstream(Utils::GetRealAssetPath(path));
    nlohmann::json json;
    s >> json;
    s.close();

    auto result = new Material();
    std::string shaderPath;
    for (const auto& elem : json.items())
    {
        const auto& elemKey = elem.key();
        const auto& elemValue = elem.value();
        
        if (elemKey == "shader")
        {
            result->shaderId = Shader::LoadFromFile(elemValue.get<std::string>());
            continue;
        }
        
        if (elemValue.is_number_integer())
        {
            result->setIntValue(elemKey, elemValue.get<int>());
            continue;
        }
        
        if (elemValue.is_number_float())
        {
            result->setFloatValue(elemKey, elemValue.get<float>());
            continue;
        }
        
        if (elemValue.is_boolean())
        {
            result->setBoolValue(elemKey, elemValue.get<bool>());
            continue;
        }
        
        if (Utils::IsVec4(elemValue))
        {
            result->setVector4Value(elemKey, Utils::ToVec4(elemValue));
            continue;
        }
        
        if (elemValue.is_string() && Utils::EndsWith(elemKey, "Tex"))
        {
            result->setTextureValue(elemKey, Texture::LoadFromFile(elemValue.get<std::string>()));
            continue;
        }
    }

    ResourceMgr::RegisterResource(path, result->id);
    Utils::Log("成功载入Material " + path);
    return result->id;
}
