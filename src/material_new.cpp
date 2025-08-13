#include "material_new.h"

#include "game_resource.h"
#include "image.h"

namespace op
{
    MaterialNew* MaterialNew::s_globalMat = nullptr;
    
    MaterialNew::MaterialNew()
    {
        m_onFrameEndHandler = GameResource::GetInstance()->onFrameEnd.Add(this, &MaterialNew::OnFrameEnd);
    }

    MaterialNew::~MaterialNew()
    {
        GameResource::GetInstance()->onFrameEnd.Remove(m_onFrameEndHandler);

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
    }

    void MaterialNew::Use(const Mesh* mesh, const RenderContext* renderContext)
    {
        if (mesh)
        {
            m_shader->Use(mesh);
        }

        if (m_shader && renderContext)
        {
            ApplyTextures(renderContext);
        }
        
        if (HasCBuffer())
        {
            SyncCBuffer(false);
            BindUbo();
        }
    }

    void MaterialNew::SetShader(Shader* shader)
    {
        if (m_shader)
        {
            throw std::runtime_error("Shader已绑定");
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

    void MaterialNew::CreateCBuffer(CBufferLayout* cbufferLayout)
    {
        if (HasCBuffer())
        {
            throw std::runtime_error("CBuffer已绑定");
        }

        m_cbuffer = new CBuffer(cbufferLayout);
        INCREF(m_cbuffer);

        SyncCBuffer(true);
    }

    MaterialNew* MaterialNew::LoadFromFile(const std::string& path)
    {
        {
            SharedObject* result;
            if(TryGetResource(path, result))
            {
                return dynamic_cast<MaterialNew*>(result);
            }
        }

        auto json = Utils::LoadJson(path);

        auto result = new MaterialNew();
        Shader* shader = nullptr;
        for (const auto& elem : json.items())
        {
            const auto& elemKey = StringHandle(elem.key());
            const auto& elemValue = elem.value();
            
            if (elemKey == "shader")
            {
                shader = Shader::LoadFromFile(elemValue.get<std::string>());
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
            
            // if (elemValue.is_number_integer())
            // {
            //     result->SetIntValue(elemKey, elemValue.get<int>());
            //     continue;
            // }
            
            if (elemValue.is_number_float())
            {
                result->Set(elemKey, elemValue.get<float>());
                continue;
            }
            
            // if (elemValue.is_boolean())
            // {
            //     result->SetBoolValue(elemKey, elemValue.get<bool>());
            //     continue;
            // }
            
            if (Utils::IsVec4(elemValue))
            {
                result->Set(elemKey, elemValue.get<Vec4>());
                continue;
            }
            
            if (elemValue.is_string() && Utils::EndsWith(elemKey, "Tex"))
            {
                auto texture = Image::LoadFromFile(elemValue.get<std::string>(), ImageDescriptor::GetDefault());
                result->Set(elemKey, texture);
                continue;
            }
        }

        RegisterResource(path, result);
        if (shader)
        {
            result->SetShader(shader);
        }
        Utils::LogInfo("成功载入Material " + path);
        return result;
    }

    void MaterialNew::SyncCBuffer(const bool force)
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

            auto size = m_valueInfos[nameId].size;
            auto srcData = Get(nameId, size);
            m_cbuffer->Sync({ srcData, param });
        }

        m_cbuffer->EndSync();
        
        Utils::CheckGlError("同步UBO数据");

        m_dirtyValues.clear();
    }
    
    void MaterialNew::BindUbo()
    {
        m_cbuffer->Use();
    }

    void MaterialNew::SetAllValuesDirty()
    {
        for (auto& [nameId, valueInfo] : m_valueInfos)
        {
            m_dirtyValues.insert(nameId);
        }
    }

    void MaterialNew::ApplyTextures(const RenderContext* renderContext)
    {
        std::vector<Texture*> needBindingTextures;
        std::vector<size_t> needBindingTexturesNameId;
        needBindingTextures.reserve(m_shader->textures.size());
        needBindingTexturesNameId.reserve(m_shader->textures.size());
        for (auto& [nameId, textureInfo] : m_shader->textures)
        {
            if (auto texture = GetTexture(nameId))
            {
                needBindingTextures.push_back(texture);
                needBindingTexturesNameId.push_back(nameId);
            }
        }

        auto bindingInfos = renderContext->textureBindingMgr->BindTextures(needBindingTextures);
        for (size_t i = 0; i < bindingInfos.size(); i++)
        {
            m_shader->SetVal(needBindingTexturesNameId[i], bindingInfos[i].slot);
        }
    }

    void MaterialNew::OnFrameEnd()
    {
        m_dirtyValues.clear();
    }
}
