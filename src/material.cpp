#include "material.h"

#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "shader.h"
#include "built_in_res.h"
#include "image.h"
#include "string_handle.h"
#include "utils.h"

namespace op
{
    Material* Material::s_globalMaterial = new Material("Global Material");

    Material::Material(const std::string& name)
    {
        this->name = name;
    }

    Material::~Material()
    {
        Clear();

        if (shader)
        {
            DECREF(shader);
        }
    }

    void Material::SetIntValue(const StringHandle& paramName, const int value)
    {
        intValues[paramName.GetHash()] = value;
    }
    
    void Material::SetBoolValue(const StringHandle& paramName, const bool value)
    {
        boolValues[paramName.GetHash()] = value;
    }
    
    void Material::SetFloatValue(const StringHandle& paramName, const float value)
    {
        floatValues[paramName.GetHash()] = value;
    }
    
    void Material::SetMat4Value(const StringHandle& paramName, const Matrix4x4& value)
    {
        mat4Values[paramName.GetHash()] = value;
    }
    
    void Material::SetVector4Value(const StringHandle& paramName, const Vec4& value)
    {
        vec4Values[paramName.GetHash()] = value;
    }
    
    void Material::SetFloatArrValue(const StringHandle& paramName, const float *value, const int count)
    {
        std::vector<float>* arr;
        auto it = floatArrValues.find(paramName.GetHash());
        if (it == floatArrValues.end())
        {
            arr = new std::vector<float>();
            floatArrValues[paramName.GetHash()] = arr;
        }
        else
        {
            arr = it->second;
        }
        arr->resize(count);
        std::memcpy(arr->data(), value, count * sizeof(float));
    }

    void Material::SetTextureValue(const StringHandle& paramName, Texture* value)
    {
        auto it = textureValues.find(paramName.GetHash());
        if (it != textureValues.end())
        {
            if (it->second.texture == value)
            {
                return;
            }
            
            DECREF(it->second.texture);
        }

        if (value)
        {
            auto texelHandle = StringHandle(paramName.GetString() + "_TexelSize");
            textureValues[paramName.GetHash()] = {value, texelHandle.GetHash()};
            INCREF(value);
            
            auto texelSize = Vec4(
                1.0f / static_cast<float>(value->width),
                1.0f / static_cast<float>(value->height),
                static_cast<float>(value->width),
                static_cast<float>(value->height));
            vec4Values[texelHandle.GetHash()] = texelSize;
        }
        else if (it != textureValues.end())
        {
            textureValues.erase(paramName.GetHash());
            vec4Values.erase(it->second.texelNameId);
        }
    }

    void Material::SetGlobalIntValue(const StringHandle& paramName, const int value)
    {
        s_globalMaterial->SetIntValue(paramName, value);
    }
    
    void Material::SetGlobalBoolValue(const StringHandle& paramName, const bool value)
    {
        s_globalMaterial->SetBoolValue(paramName, value);
    }
    
    void Material::SetGlobalFloatValue(const StringHandle& paramName, const float value)
    {
        s_globalMaterial->SetFloatValue(paramName, value);
    }

    void Material::SetGlobalMat4Value(const StringHandle& paramName, const Matrix4x4& value)
    {
        s_globalMaterial->SetMat4Value(paramName, value);
    }

    void Material::SetGlobalVector4Value(const StringHandle& paramName, const Vec4& value)
    {
        s_globalMaterial->SetVector4Value(paramName, value);
    }

    void Material::SetGlobalFloatArrValue(const StringHandle& paramName, const float* value, const int count)
    {
        s_globalMaterial->SetFloatArrValue(paramName, value, count);
    }

    void Material::SetGlobalTextureValue(const std::string& paramName, Texture* value)
    {
        s_globalMaterial->SetTextureValue(paramName, value);
    }

    void Material::ClearAllGlobalValues()
    {
        s_globalMaterial->Clear();
    }

    void Material::ReleaseStaticRes()
    {
        delete s_globalMaterial;
        s_globalMaterial = nullptr;
    }

    void Material::FillParams(const Shader* targetShader) const
    {
        auto slot = 0;
        for (const auto& [nameId, uniformInfo] : targetShader->uniforms)
        {
            switch (uniformInfo.type)
            {
            case GL_FLOAT:
                {
                    if (uniformInfo.elemNum > 1)
                    {
                        std::vector<float>* fa;
                        if (FindParam(nameId, floatArrValues, s_globalMaterial->floatArrValues, fa) && !fa->empty())
                        {
                            Shader::SetFloatArrGl(uniformInfo.location, fa->size(), fa->data());
                        }
                    }
                    else
                    {
                        float f = 0;
                        FindParam(nameId, floatValues, s_globalMaterial->floatValues, f);
                        Shader::SetFloatGl(uniformInfo.location, f);
                    }
                    break;
                }
                
            case GL_INT:
                {
                    auto i = 0;
                    FindParam(nameId, intValues, s_globalMaterial->intValues, i);
                    Shader::SetIntGl(uniformInfo.location, i);
                    break;
                }
                
            case GL_BOOL:
                {
                    auto b = false;
                    FindParam(nameId, boolValues, s_globalMaterial->boolValues, b);
                    Shader::SetBoolGl(uniformInfo.location, b);
                    break;
                }
                
            case GL_FLOAT_MAT4:
                {
                    Matrix4x4 m;
                    FindParam(nameId, mat4Values, s_globalMaterial->mat4Values, m);
                    Shader::SetMatrixGl(uniformInfo.location, m);
                    break;
                }

            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_2D:
                {
                    MaterialTextureValue t;
                    if (!FindParam(nameId, textureValues, s_globalMaterial->textureValues, t))
                    {
                        t.texture = BuiltInRes::GetInstance()->errorTex;
                    }
                    Shader::SetTextureGl(uniformInfo.location, slot++, t.texture);
                    break;
                }
                
            case GL_FLOAT_VEC4:
                {
                    Vec4 v;
                    FindParam(nameId, vec4Values, s_globalMaterial->vec4Values, v);
                    Shader::SetVectorGl(uniformInfo.location, v);
                    break;
                }

            default:
                {
                    throw std::runtime_error("不支持的Uniform类型：" + std::to_string(uniformInfo.type));
                }
            }
        }
    }

    void Material::Use(const Mesh* mesh) const
    {
        if(shader == nullptr)
        {
            throw std::runtime_error((std::stringstream() << "材质 " << name << " 未加载Shader").str());
        }

        shader->Use(mesh);
        FillParams(shader); // cbuffer 优化
    }

    void Material::Clear()
    {
        intValues.clear();
        boolValues.clear();
        floatValues.clear();
        vec4Values.clear();
        mat4Values.clear();
        for (auto pair : floatArrValues)
        {
            delete pair.second;
        }
        floatArrValues.clear();
        for (auto pair : textureValues)
        {
            DECREF(pair.second.texture);
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
        
        auto s = std::ifstream(Utils::GetAbsolutePath(path));
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

            if (elemKey == "blendMode")
            {
                result->blendMode = BlendModeMgr::FromStr(elemValue.get<std::string>());
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
                result->SetVector4Value(elemKey, elemValue.get<Vec4>());
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
}
