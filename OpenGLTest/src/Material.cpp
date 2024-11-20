#include "Material.h"
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
    if(ResourceMgr::HasResourceRegistered(path))
    {
        return ResourceMgr::GetRegisteredResource(path);
    }
    
    auto s = std::ifstream(path);
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
        
        if(elemKey == "vert")
        {
            vertShaderPath = elemValue.get<std::string>();
        }
        else if(elemKey == "frag")
        {
            fragShaderPath = elemValue.get<std::string>();
        }
        else if(elem.value().is_number_integer())
        {
            result->SetIntValue(elemKey, elemValue.get<int>());
        }
        else if(elemValue.is_number_float())
        {
            result->SetFloatValue(elemKey, elemValue.get<float>());
        }
        else if(elemValue.is_boolean())
        {
            result->SetBoolValue(elemKey, elemValue.get<bool>());
        }
        else
        {
            std::string textureSuffix = "Tex";
            // ends with suffix
            if(elemKey.size() >= textureSuffix.size() && elemKey.rfind(textureSuffix) == elemKey.size() - textureSuffix.size())
            {
                result->SetTextureValue(elemKey, Texture::LoadFromFile(elemValue.get<std::string>()));
            }
        }
    }

    if(!vertShaderPath.empty() && !fragShaderPath.empty())
    {
        result->m_shader = Shader::LoadFromFile(vertShaderPath, fragShaderPath);
    }

    ResourceMgr::RegisterResource(path, result->m_id);
    return result->m_id;
}

Material::Material()
{
    m_id = ResourceMgr::AddPtr(this);
}

Material::~Material()
{
    ResourceMgr::RemovePtr(m_id);
}
