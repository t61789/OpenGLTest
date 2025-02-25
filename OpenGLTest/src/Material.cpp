#include "Material.h"

#include <fstream>
#include <sstream>

#include "Image.h"
#include "Utils.h"
#include "../lib/json.hpp"

Material* Material::s_globalMaterial = new Material("Global Material");
Material* Material::s_tempMaterial = new Material("Temp Material");

Material::Material(const std::string& name)
{
    this->name = name;
}

Material::~Material()
{
    if (this == s_tempMaterial)
    {
        return;
    }
    
    for (auto& element : floatArrValues)
    {
        delete element.second;
    }

    for (auto& element : textureValues)
    {
        DECREF(element.second);
    }

    if (shader)
    {
        DECREF(shader);
    }
}

void Material::SetIntValue(const std::string& paramName, const int value)
{
    intValues[paramName] = value;
}

void Material::SetBoolValue(const std::string& paramName, const bool value)
{
    boolValues[paramName] = value;
}

void Material::SetFloatValue(const std::string& paramName, const float value)
{
    floatValues[paramName] = value;
}

void Material::SetMat4Value(const std::string& paramName, const glm::mat4& value)
{
    mat4Values[paramName] = value;
}

void Material::SetTextureValue(const std::string& paramName, Texture* value)
{
    auto it = textureValues.find(paramName);
    if (it != textureValues.end())
    {
        if (it->second == value)
        {
            return;
        }
        
        DECREF(it->second);
    }
    
    if (value)
    {
        textureValues[paramName] = value;
        INCREF(value);
    }
}

void Material::SetVector4Value(const std::string& paramName, const glm::vec4& value)
{
    vec4Values[paramName] = value;
}

void Material::SetFloatArrValue(const std::string& paramName, const float *value, const int count)
{
    auto newValue = new std::vector<float>(value, value + count);
    floatArrValues[paramName] = newValue;
}

void Material::SetGlobalIntValue(const std::string& paramName, const int value)
{
    s_globalMaterial->SetIntValue(paramName, value);
}

void Material::SetGlobalBoolValue(const std::string& paramName, const bool value)
{
    s_globalMaterial->SetBoolValue(paramName, value);
}

void Material::SetGlobalFloatValue(const std::string& paramName, const float value)
{
    s_globalMaterial->SetFloatValue(paramName, value);
}

void Material::SetGlobalMat4Value(const std::string& paramName, const glm::mat4& value)
{
    s_globalMaterial->SetMat4Value(paramName, value);
}

void Material::SetGlobalTextureValue(const std::string& paramName, Texture* value)
{
    s_globalMaterial->SetTextureValue(paramName, value);
}

void Material::SetGlobalVector4Value(const std::string& paramName, const glm::vec4& value)
{
    s_globalMaterial->SetVector4Value(paramName, value);
}

void Material::SetGlobalFloatArrValue(const std::string& paramName, const float* value, const int count)
{
    s_globalMaterial->SetFloatArrValue(paramName, value, count);
}

void Material::ClearAllGlobalValues()
{
    s_globalMaterial->Clear();
}

void Material::ReleaseStaticRes()
{
    delete s_globalMaterial;
    s_globalMaterial = nullptr;
    delete s_tempMaterial;
    s_tempMaterial = nullptr;
}

void Material::FillParams(const Shader* targetShader) const
{
    // ---------------------将全局参数写入临时材质---------------------
    s_tempMaterial->intValues = s_globalMaterial->intValues;
    s_tempMaterial->boolValues = s_globalMaterial->boolValues;
    s_tempMaterial->floatValues = s_globalMaterial->floatValues;
    s_tempMaterial->mat4Values = s_globalMaterial->mat4Values;
    s_tempMaterial->textureValues = s_globalMaterial->textureValues;
    s_tempMaterial->vec4Values = s_globalMaterial->vec4Values;
    s_tempMaterial->floatArrValues = s_globalMaterial->floatArrValues;
    
    // ---------------------使用材质参数覆盖全局参数---------------------
    for (const auto& element : intValues)
    {
        s_tempMaterial->intValues[element.first] = element.second;
    }
    for (const auto& element : boolValues)
    {
        s_tempMaterial->boolValues[element.first] = element.second;
    }
    for (const auto& element : floatValues)
    {
        s_tempMaterial->floatValues[element.first] = element.second;
    }
    for (const auto& element : mat4Values)
    {
        s_tempMaterial->mat4Values[element.first] = element.second;
    }
    for (const auto& element : textureValues)
    {
        s_tempMaterial->textureValues[element.first] = element.second;
    }
    for (const auto& element : vec4Values)
    {
        s_tempMaterial->vec4Values[element.first] = element.second;
    }
    for (const auto& element : floatArrValues)
    {
        s_tempMaterial->floatArrValues[element.first] = element.second;
    }
    
    // ---------------------实际写入数据---------------------
    for (const auto& element : s_tempMaterial->intValues)
    {
        targetShader->SetInt(element.first, element.second);
    }
    for (const auto& element : s_tempMaterial->boolValues)
    {
        targetShader->SetBool(element.first, element.second);
    }
    for (const auto& element : s_tempMaterial->floatValues)
    {
        targetShader->SetFloat(element.first, element.second);
    }
    for (const auto& element : s_tempMaterial->vec4Values)
    {
        targetShader->SetVector(element.first, element.second);
    }
    for (const auto& element : s_tempMaterial->mat4Values)
    {
        targetShader->SetMatrix(element.first, element.second);
    }
    for (const auto& element : s_tempMaterial->floatArrValues)
    {
        targetShader->SetFloatArr(element.first, static_cast<uint32_t>(element.second->size()), element.second->data());
    }
    int slot = 0;
    for (auto& element : s_tempMaterial->textureValues)
    {
        if(!targetShader->HasParam(element.first))
        {
            continue;
        }
        
        targetShader->SetTexture(element.first, slot, element.second);
        auto texelSize = glm::vec4(
            1.0f / static_cast<float>(element.second->width),
            1.0f / static_cast<float>(element.second->height),
            static_cast<float>(element.second->width),
            static_cast<float>(element.second->height));
        targetShader->SetVector(element.first + std::string("_TexelSize"), texelSize);
        slot++;
    }
}

void Material::Use(const Mesh* mesh) const
{
    if(shader == nullptr)
    {
        throw std::runtime_error((std::stringstream() << "材质 " << name << " 未加载Shader").str());
    }

    shader->Use(mesh);
    FillParams(shader);
}

void Material::Clear()
{
    intValues.clear();
    boolValues.clear();
    floatValues.clear();
    vec4Values.clear();
    mat4Values.clear();
    for (const auto& pair : floatArrValues)
    {
        delete pair.second;
    }
    floatArrValues.clear();
    for (auto pair : textureValues)
    {
        DECREF(pair.second);
    }
    textureValues.clear();
}

Material* Material::LoadFromFile(const std::string& path)
{
    {
        SharedObject* result;
        if(TryGetResource(path, result))
        {
            return dynamic_cast<Material*>(result);
        }
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
            result->shader = Shader::LoadFromFile(elemValue.get<std::string>());
            INCREF_BY(result->shader, result);
            continue;
        }

        if (elemKey == "cullMode")
        {
            result->cullMode = CullModeMgr::FromStr(elemValue.get<std::string>());
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
            result->SetTextureValue(elemKey, Image::LoadFromFile(elemValue.get<std::string>(), ImageDescriptor::GetDefault()));
            continue;
        }
    }

    auto pos = path.find_last_of("/\\");
    auto filename = pos != std::string::npos ? path.substr(pos + 1) : path;
    result->name = filename;
    RegisterResource(path, result);
    Utils::LogInfo("成功载入Material " + path);
    return result;
}

Material* Material::CreateEmptyMaterial(const std::string& shaderPath, const std::string& name)
{
    auto shader = Shader::LoadFromFile(shaderPath);
    if(shader == nullptr)
    {
        return nullptr;
    }
    
    auto result = new Material();
    result->shader = shader;
    INCREF_BY(result->shader, result);
    result->name = name;
    return result;
}
