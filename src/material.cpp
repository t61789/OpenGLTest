#include "material.h"

#include "built_in_res.h"
#include "game_resource.h"
#include "image.h"

namespace op
{
    Material::~Material()
    {
        if (m_shader)
        {
            DECREF(m_shader);
        }

        if (m_cbuffer)
        {
            DECREF(m_cbuffer);
        }

        for (auto& [nameId, texture] : m_textures)
        {
            DECREF(texture);
        }

        for (auto& valueInfo : m_values)
        {
            delete[] valueInfo.data;
        }
    }

    void Material::BindShader(Shader* shader)
    {
        if (m_shader)
        {
            THROW_ERROR("Shader已绑定")
        }

        m_shader = shader;
        INCREF(m_shader);

        if (shader->cbuffers.empty())
        {
            return;
        }

        for (auto& [nameId, cbuffer] : shader->cbuffers)
        {
            // 就绑定第一个，现在除了预定义的CBuffer外仅支持一个CBuffer
            CreateCBuffer(cbuffer);
            break;
        }
    }

    void Material::CreateCBuffer(CBufferLayout* cbufferLayout)
    {
        if (HasCBuffer())
        {
            THROW_ERROR("CBuffer已绑定")
        }

        m_cbuffer = new CBuffer(cbufferLayout);
        INCREF(m_cbuffer);

        SyncCBuffer(true);
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

        auto json = Utils::LoadJson(path);

        auto result = new Material();
        Shader* shader = nullptr;
        for (const auto& elem : json.items())
        {
            const auto& elemKey = StringHandle(elem.key());
            const auto& elemValue = elem.value();
            
            if (elemKey.Str() == "shader")
            {
                shader = Shader::LoadFromFile(elemValue.get<std::string>());
                continue;
            }

            if (elemKey.Str() == "cullMode")
            {
                result->cullMode = RenderState::CullModeFromStr(elemValue.get<std::string>());
                continue;
            }

            if (elemKey.Str() == "blendMode")
            {
                result->blendMode = RenderState::BlendModeFromStr(elemValue.get<std::string>());
                continue;
            }
            
            if (elemValue.is_number_integer())
            {
                result->Set(elemKey, elemValue.get<int>());
                continue;
            }
            
            if (elemValue.is_number_float())
            {
                result->Set(elemKey, elemValue.get<float>());
                continue;
            }
            
            if (elemValue.is_boolean())
            {
                result->Set(elemKey, elemValue.get<bool>());
                continue;
            }
            
            if (is_vec4(elemValue))
            {
                result->Set(elemKey, elemValue.get<Vec4>());
                continue;
            }
            
            if (elemValue.is_string() && ends_with(elemKey, "Tex"))
            {
                auto texture = Image::LoadFromFile(elemValue.get<std::string>(), ImageDescriptor::GetDefault());
                result->Set(elemKey, texture);
                continue;
            }
        }

        RegisterResource(path, result);
        if (shader)
        {
            result->BindShader(shader);
        }
        log_info("成功载入Material " + path);
        return result;
    }

    Material* Material::CreateFromShader(const std::string& path)
    {
        auto mat = new Material();
        auto shader = Shader::LoadFromFile(path);
        mat->BindShader(shader);
        return mat;
    }

    void Material::UseCBuffer()
    {
        if (!HasCBuffer())
        {
            return;
        }

        SyncCBuffer(false);
        BindUbo();
    }

    void Material::SyncCBuffer(const bool force)
    {
        if (force)
        {
            SetAllValuesDirty();
        }

        if (m_dirtyValues.empty())
        {
            return;
        }

        m_cbuffer->StartSync();
        
        for (auto& nameId : m_dirtyValues)
        {
            CBufferParam* param;
            if (!m_cbuffer->TryGetParam(nameId, &param))
            {
                continue;
            }

            auto valueInfo = find(m_values, &ValueInfo::nameId, nameId);
            m_cbuffer->Sync(valueInfo->data, param->offset, std::min(param->size, static_cast<size_t>(valueInfo->byteCount)));
        }

        // m_cbuffer->EndSync();
        
        GL_CHECK_ERROR(同步UBO数据)

        m_dirtyValues.clear();
    }
    
    void Material::BindUbo()
    {
        m_cbuffer->Use();
    }

    void Material::SetAllValuesDirty()
    {
        for (auto& valueInfo : m_values)
        {
            m_dirtyValues.insert(valueInfo.nameId);
        }
    }

    void Material::OnFrameEnd()
    {
        // m_dirtyValues.clear();
    }
}
