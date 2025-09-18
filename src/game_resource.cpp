#include "game_resource.h"
#include "const.h"
#include "material.h"
#include "render/gl/gl_cbuffer.h"

namespace op
{
    GameResource::GameResource()
    {
        m_perObjectBuffer = mup<PerObjectBuffer>(5000, 4);
        m_globalTextureSet = mup<TextureSet>();
        for (auto& matName : PREDEFINED_MATERIALS)
        {
            m_predefinedCbuffers[matName] = nullptr;
        }
    }

    GlCbuffer* GameResource::GetPredefinedCbuffer(const size_t nameId)
    {
        auto it = m_predefinedCbuffers.find(nameId);
        if (it != m_predefinedCbuffers.end())
        {
            return it->second.get();
        }

        return nullptr;
    }

    bool GameResource::IsPredefinedCbuffer(const size_t nameId)
    {
        return m_predefinedCbuffers.find(nameId) != m_predefinedCbuffers.end();
    }

    bool GameResource::NeedCreatePredefinedCbuffer(const size_t nameId)
    {
        auto it = m_predefinedCbuffers.find(nameId);
        if (it == m_predefinedCbuffers.end() || it->second)
        {
            return false;
        }

        return it->second == nullptr;
    }
    
    void GameResource::CreatePredefinedCbuffer(const size_t nameId, crsp<CBufferLayout> layout)
    {
        auto it = m_predefinedCbuffers.find(nameId);
        if (it == m_predefinedCbuffers.end() || it->second)
        {
            return;
        }

        it->second = mup<GlCbuffer>(layout);

        return;
    }

    void GameResource::RegisterResource(cr<StringHandle> pathId, crsp<IResource> resource)
    {
        assert(!GetResource(pathId));
        
        m_resources[pathId] = resource;
    }

    void GameResource::UnregisterResource(cr<StringHandle> pathId)
    {
        assert(GetResource(pathId));
        
        m_resources.erase(pathId);
    }

    sp<IResource> GameResource::GetResource(cr<StringHandle> pathId)
    {
        auto it = m_resources.find(pathId);
        if (it == m_resources.end())
        {
            return nullptr;
        }

        if (it->second.expired())
        {
            m_resources.erase(it);
            return nullptr;
        }

        return it->second.lock();
    }
}
