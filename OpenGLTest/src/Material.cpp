#include "Material.h"

#include "Utils.h"
#include "../lib/json.hpp"

void Material::SetIntValue(const std::string& name, const int value)
{
    m_intValues[name] = value;
}

void Material::SetBoolValue(const std::string& name, const bool value)
{
    m_boolValues[name] = value;
}

void Material::SetFloatValue(const std::string& name, const float value)
{
    m_floatValues[name] = value;
}

void Material::SetMat4Value(const std::string& name, const glm::mat4& value)
{
    m_mat4Values[name] = value;
}

void Material::SetTextureValue(const std::string& name, RESOURCE_ID value)
{
    m_textureValues[name] = value;
}

void Material::SetVector4Value(const std::string& name, const glm::vec4& value)
{
    m_vec4Values[name] = value;
}

void Material::FillParams(const Shader* shader) const
{
    for (const auto& element : m_intValues)
    {
        shader->SetInt(element.first, element.second);
    }
    
    for (const auto& element : m_boolValues)
    {
        shader->SetBool(element.first, element.second);
    }
    
    for (const auto& element : m_floatValues)
    {
        shader->SetFloat(element.first, element.second);
    }
    
    for (const auto& element : m_vec4Values)
    {
        shader->SetVector(element.first, element.second);
    }
    
    for (const auto& element : m_mat4Values)
    {
        shader->SetMatrix(element.first, element.second);
    }

    int slot = 0;
    for (auto& element : m_textureValues)
    {
        if(!shader->HasParam(element.first))
        {
            return;
        }
        
        shader->SetTexture(element.first, slot, element.second);
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
    std::string vertShaderPath;
    std::string fragShaderPath;
    for (const auto& elem : json.items())
    {
        const auto& elemKey = elem.key();
        const auto& elemValue = elem.value();
        
        if (elemKey == "vert")
        {
            vertShaderPath = elemValue.get<std::string>();
            continue;
        }
        
        if (elemKey == "frag")
        {
            fragShaderPath = elemValue.get<std::string>();
            continue;
        }
        
        if (elemValue.is_number_integer())
        {
            result->SetIntValue(elemKey, elemValue.get<int>());
            continue;
        }
        
        if (elemValue.is_number_float())
        {
            result->SetFloatValue(elemKey, elemValue.get<float>());
            continue;
        }
        
        if (elemValue.is_boolean())
        {
            result->SetBoolValue(elemKey, elemValue.get<bool>());
            continue;
        }
        
        if (Utils::IsVec4(elemValue))
        {
            result->SetVector4Value(elemKey, Utils::ToVec4(elemValue));
            continue;
        }
        
        if (elemValue.is_string() && Utils::EndsWith(elemKey, "Tex"))
        {
            result->SetTextureValue(elemKey, Texture::LoadFromFile(elemValue.get<std::string>()));
            continue;
        }
    }

    if(!vertShaderPath.empty() && !fragShaderPath.empty())
    {
        result->m_shader = Shader::LoadFromFile(vertShaderPath, fragShaderPath);
    }

    ResourceMgr::RegisterResource(path, result->m_id);
    Utils::LogInfo("成功载入Material " + path);
    return result->m_id;
}
