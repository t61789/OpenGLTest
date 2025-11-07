#include "material.h"

#include "game_resource.h"
#include "image.h"
#include "shader.h"
#include "common/data_set.h"
#include "render/shader_variants.h"
#include "render/texture_set.h"
#include "render/gl/gl_cbuffer.h"
#include "render/gl/gl_state.h"

namespace op
{
    Material::Material()
    {
        cullMode = CullMode::BACK;
        blendMode = BlendMode::NONE;
        depthMode = DepthMode::LESS_EQUAL;
        depthWrite = true;
        m_dataSet = mup<DataSet>();
        m_textureSet = mup<TextureSet>();
    }

    void Material::SetTexture(const size_t nameId, crsp<ITexture> val)
    {
        m_textureSet->SetTexture(nameId, val);
    }

    void Material::Set(const size_t nameId, const float* val, const size_t count)
    {
        TrySetImp(nameId, val, count * sizeof(float));
    }

    void Material::Get(const size_t nameId, float* val, const size_t count)
    {
        TryGetImp(nameId, val, count * sizeof(float));
    }

    sp<ITexture> Material::GetTexture(const size_t nameId)
    {
        return m_textureSet->GetTexture(nameId);
    }

    void Material::BindShader(crsp<Shader> shader)
    {
        if (m_shader)
        {
            THROW_ERROR("Shader已绑定")
        }

        m_shader = shader;

        if (shader->cbuffers.empty())
        {
            return;
        }

        for (const auto& [nameId, cbuffer] : shader->cbuffers)
        {
            // 就绑定第一个，现在除了预定义的CBuffer外仅支持一个CBuffer
            CreateCBuffer(cbuffer);
            break;
        }
    }

    void Material::CreateCBuffer(crsp<CBufferLayout> cbufferLayout)
    {
        if (HasCBuffer())
        {
            THROW_ERROR("CBuffer已绑定")
        }

        m_cbuffer = mup<GlCbuffer>(cbufferLayout);

        // 把存在dataSet里的数据放到Cbuffer里去，放不进去就仍然放dataSet里
        
        auto allData = m_dataSet->GetAllData();
        if (allData.empty())
        {
            return;
        }

        vec<string_hash> inCbufferParams;
        inCbufferParams.reserve(allData.size());

        for (auto& dataInfo : allData)
        {
            if (m_cbuffer->TrySetRaw(dataInfo.nameId, dataInfo.data, dataInfo.sizeB))
            {
                inCbufferParams.push_back(dataInfo.nameId);
            }
        }

        for (auto& nameId : inCbufferParams)
        {
            m_dataSet->Remove(nameId);
        }
    }

    sp<Material> Material::LoadFromFile(cr<StringHandle> path)
    {
        {
            if (auto result = GetGR()->GetResource<Material>(path))
            {
                return result;
            }
        }

        auto json = Utils::LoadJson(path);

        auto result = msp<Material>();
        sp<Shader> shader = nullptr;
        for (const auto& elem : json.items())
        {
            const auto& elemKey = StringHandle(elem.key());
            const auto& elemValue = elem.value();
            
            if (elemKey.Str() == "shader")
            {
                shader = ShaderVariants::LoadFromFile(elemValue.get<std::string>())->GetShader();
                continue;
            }

            if (elemKey.Str() == "cullMode")
            {
                result->cullMode = GlState::GetCullMode(elemValue.get<std::string>());
                continue;
            }

            if (elemKey.Str() == "blendMode")
            {
                result->blendMode = GlState::GetBlendMode(elemValue.get<std::string>());
                continue;
            }

            if (elemKey.Str() == "depthMode")
            {
                result->depthMode = GlState::GetDepthMode(elemValue.get<std::string>());
                continue;
            }
            
            if (elemKey.Str() == "depthWrite")
            {
                result->depthWrite = elemValue.get<bool>();
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
                auto texture = Image::LoadFromFile(elemValue.get<std::string>());
                result->SetTexture(elemKey, texture);
                continue;
            }
        }

        GetGR()->RegisterResource(path, result);
        result->m_path = path;
        
        if (shader)
        {
            result->BindShader(shader);
        }
        
        log_info("Load material: %s", path.CStr());
        
        return result;
    }

    sp<Material> Material::CreateFromShader(cr<StringHandle> path)
    {
        auto mat = msp<Material>();
        auto shader = ShaderVariants::LoadFromFile(path)->GetShader();
        mat->BindShader(shader);
        return mat;
    }

    void Material::UseCBuffer()
    {
        if (!HasCBuffer())
        {
            return;
        }

        m_cbuffer->BindBase();
    }

    void Material::OnFrameEnd()
    {
        // m_dirtyValues.clear();
    }

    bool Material::TrySetImp(const string_hash nameId, const void* data, const uint32_t sizeB)
    {
        if (HasCBuffer())
        {
            return m_cbuffer->TrySetRaw(nameId, data, sizeB);
        }

        return m_dataSet->TrySetImp(nameId, data, sizeB);
    }
    
    bool Material::TryGetImp(const string_hash nameId, void* data, const uint32_t sizeB)
    {
        if (HasCBuffer() && m_cbuffer->TryGetRaw(nameId, data, sizeB))
        {
            return true;
        }

        if (m_dataSet->TryGetImp(nameId, data, sizeB))
        {
            return true;
        }

        return false;
    }
}
