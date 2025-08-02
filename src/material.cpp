#include "material.h"

#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "shader.h"
#include "built_in_res.h"
#include "image.h"
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

    void Material::SetMat4Value(const std::string& paramName, const Matrix4x4& value)
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
            
            auto texelSize = Vec4(
                1.0f / static_cast<float>(value->width),
                1.0f / static_cast<float>(value->height),
                static_cast<float>(value->width),
                static_cast<float>(value->height));
            vec4Values[paramName + "_TexelSize"] = texelSize;
        }
        else if (it != textureValues.end())
        {
            textureValues.erase(paramName);
            vec4Values.erase(paramName + "_TexelSize");
        }
    }

    void Material::SetVector4Value(const std::string& paramName, const Vec4& value)
    {
        vec4Values[paramName] = value;
    }

    void Material::SetFloatArrValue(const std::string& paramName, const float *value, const int count)
    {
        std::vector<float>* arr;
        auto it = floatArrValues.find(paramName);
        if (it != floatArrValues.end())
        {
            arr = it->second;
        }
        else
        {
            arr = new std::vector<float>();
            floatArrValues[paramName] = arr;
        }
        arr->resize(count);
        std::memcpy(arr->data(), value, count * sizeof(float));
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

    void Material::SetGlobalMat4Value(const std::string& paramName, const Matrix4x4& value)
    {
        s_globalMaterial->SetMat4Value(paramName, value);
    }

    void Material::SetGlobalTextureValue(const std::string& paramName, Texture* value)
    {
        s_globalMaterial->SetTextureValue(paramName, value);
    }

    void Material::SetGlobalVector4Value(const std::string& paramName, const Vec4& value)
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
    }

    void Material::FillParams(const Shader* targetShader) const
    {
        auto slot = 0;
        for (const auto& uniformInfo : targetShader->uniforms)
        {
            switch (uniformInfo.type)
            {
            case GL_FLOAT:
                {
                    if (uniformInfo.elemNum > 1)
                    {
                        std::vector<float>* fa;
                        auto n = uniformInfo.name.substr(0, uniformInfo.name.length() - 3);
                        if (FindParam(n, floatArrValues, s_globalMaterial->floatArrValues, fa) && !fa->empty())
                        {
                            targetShader->SetFloatArr(n, uniformInfo.elemNum, fa->data());
                        }
                    }
                    else
                    {
                        float f = 0;
                        FindParam(uniformInfo.name, floatValues, s_globalMaterial->floatValues, f);
                        targetShader->SetFloat(uniformInfo.name, f);
                    }
                    break;
                }
                
            case GL_INT:
                {
                    auto i = 0;
                    FindParam(uniformInfo.name, intValues, s_globalMaterial->intValues, i);
                    targetShader->SetInt(uniformInfo.name, i);
                    break;
                }
                
            case GL_BOOL:
                {
                    auto b = false;
                    FindParam(uniformInfo.name, boolValues, s_globalMaterial->boolValues, b);
                    targetShader->SetBool(uniformInfo.name, b);
                    break;
                }
                
            case GL_FLOAT_MAT4:
                {
                    Matrix4x4 m;
                    FindParam(uniformInfo.name, mat4Values, s_globalMaterial->mat4Values, m);
                    targetShader->SetMatrix(uniformInfo.name, m);
                    break;
                }

            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_2D:
                {
                    Texture* t = nullptr;
                    if (!FindParam(uniformInfo.name, textureValues, s_globalMaterial->textureValues, t))
                    {
                        t = BuiltInRes::GetInstance()->errorTex;
                    }
                    targetShader->SetTexture(uniformInfo.name, slot++, t);
                    break;
                }
                
            case GL_FLOAT_VEC4:
                {
                    Vec4 v;
                    FindParam(uniformInfo.name, vec4Values, s_globalMaterial->vec4Values, v);
                    targetShader->SetVector(uniformInfo.name, v);
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
