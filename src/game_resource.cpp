#include "game_resource.h"
#include "const.h"
#include "material.h"
#include "scene.h"
#include "render/gl/gl_cbuffer.h"
#include "culling_system.h"

namespace op
{
    GameResource::GameResource()
    {
        m_perObjectBuffer = mup<PerObjectBuffer>(5000, 4);
        for (auto& matName : PREDEFINED_MATERIALS)
        {
            m_predefinedCbuffers[matName] = nullptr;
        }
        m_builtInRes = mup<BuiltInRes>();
        m_globalTextureSet = mup<TextureSet>();
        m_cullingSystem = msp<CullingSystem>();
        
        m_batchRenderUnit = mup<BatchRenderUnit>();
        // m_mainScene = Scene::LoadScene("scenes/test_scene/test_scene.json");
        // m_mainScene = Scene::LoadScene("scenes/rpgpp_lt_scene_1.0/scene.json");
        // m_scene = Scene::LoadScene("scenes/ImportTest/scene.json");
        m_mainScene = Scene::LoadScene("scenes/Scene_A/scene.json");
        // m_scene = Scene::LoadScene("scenes/HDRP_template/scene.json");
    }

    GameResource::~GameResource()
    {
        m_mainScene.reset();
        m_batchRenderUnit.reset();
        m_cullingSystem.reset();
        m_globalTextureSet.reset();
        m_builtInRes.reset();
        m_predefinedCbuffers.clear();
        m_perObjectBuffer.reset();
    }

    GlCbuffer* GameResource::GetPredefinedCbuffer(const size_t nameId)
    {
        return m_predefinedCbuffers.at(nameId).get();
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
        m_predefinedCbuffers[nameId] = mup<GlCbuffer>(layout);
    }

    void GameResource::RegisterResource(cr<StringHandle> path, crsp<IResource> resource)
    {
        assert(!GetResource(path));
        
        m_resources[path] = resource;
    }

    void GameResource::UnregisterResource(cr<StringHandle> path)
    {
        assert(GetResource(path));
        
        m_resources.erase(path);
    }

    sp<IResource> GameResource::GetResource(cr<StringHandle> path)
    {
        auto it = m_resources.find(path);
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
