#include "Material.h"

#include <fstream>
#include <sstream>

#include "Image.h"
#include "Utils.h"
#include "../lib/json.hpp"

std::unordered_map<std::string, int> Material::s_globalIntValues;
std::unordered_map<std::string, bool> Material::s_globalBoolValues;
std::unordered_map<std::string, float> Material::s_globalFloatValues;
std::unordered_map<std::string, glm::mat4> Material::s_globalMat4Values;
std::unordered_map<std::string, RESOURCE_ID> Material::s_globalTextureValues;
std::unordered_map<std::string, glm::vec4> Material::s_globalVec4Values;

void Material::setIntValue(const std::string& paramName, const int value)
{
    intValues[paramName] = value;
}

void Material::setBoolValue(const std::string& paramName, const bool value)
{
    boolValues[paramName] = value;
}

void Material::setFloatValue(const std::string& paramName, const float value)
{
    floatValues[paramName] = value;
}

void Material::setMat4Value(const std::string& paramName, const glm::mat4& value)
{
    mat4Values[paramName] = value;
}

void Material::setTextureValue(const std::string& paramName, const RESOURCE_ID value)
{
    textureValues[paramName] = value;
}

void Material::setVector4Value(const std::string& paramName, const glm::vec4& value)
{
    vec4Values[paramName] = value;
}

void Material::SetGlobalIntValue(const std::string& paramName, const int value)
{
    s_globalIntValues[paramName] = value;
}

void Material::SetGlobalBoolValue(const std::string& paramName, const bool value)
{
    s_globalBoolValues[paramName] = value;
}

void Material::SetGlobalFloatValue(const std::string& paramName, const float value)
{
    s_globalFloatValues[paramName] = value;
}

void Material::SetGlobalMat4Value(const std::string& paramName, const glm::mat4& value)
{
    s_globalMat4Values[paramName] = value;
}

void Material::SetGlobalTextureValue(const std::string& paramName, RESOURCE_ID value)
{
    s_globalTextureValues[paramName] = value;
}

void Material::SetGlobalVector4Value(const std::string& paramName, const glm::vec4& value)
{
    s_globalVec4Values[paramName] = value;
}

void Material::fillParams(const Shader* shader) const
{
    auto curTextureSlot = FillGlobalParams(shader);
    
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

    int slot = curTextureSlot;
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

void Material::use(const Mesh* mesh) const
{
    auto shader = ResourceMgr::GetPtr<Shader>(shaderId);
    if(shader == nullptr)
    {
        throw std::runtime_error((std::stringstream() << "材质 " << name << " 未加载Shader").str());
    }

    shader->use(mesh);
    fillParams(shader);
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
            result->setTextureValue(elemKey, Image::LoadFromFile(elemValue.get<std::string>(), ImageDescriptor::GetDefault()));
            continue;
        }
    }

    result->name = path;
    ResourceMgr::RegisterResource(path, result->id);
    Utils::LogInfo("成功载入Material " + path);
    return result->id;
}

RESOURCE_ID Material::CreateEmptyMaterial(const std::string& shaderPath)
{
    auto shader = Shader::LoadFromFile(shaderPath);
    if(shader == UNDEFINED_RESOURCE)
    {
        return UNDEFINED_RESOURCE;
    }
    
    auto result = new Material();
    result->shaderId = shader;
    return result->id;
}

int Material::FillGlobalParams(const Shader* shader)
{
    for (const auto& element : s_globalIntValues)
    {
        shader->setInt(element.first, element.second);
    }

    for (const auto& element : s_globalBoolValues)
    {
        shader->setBool(element.first, element.second);
    }

    for (const auto& element : s_globalFloatValues)
    {
        shader->setFloat(element.first, element.second);
    }

    for (const auto& element : s_globalVec4Values)
    {
        shader->setVector(element.first, element.second);
    }

    for (const auto& element : s_globalMat4Values)
    {
        shader->setMatrix(element.first, element.second);
    }

    int globalSlot = 0;
    for (const auto& element : s_globalTextureValues)
    {
        if(!shader->hasParam(element.first))
        {
            continue;
        }
        
        shader->setTexture(element.first, globalSlot, element.second);
        globalSlot++;
    }
    return globalSlot;
}
