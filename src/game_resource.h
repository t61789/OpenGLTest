#pragma once
#include "i_resource.h"
#include "utils.h"
#include "render/per_object_buffer.h"
#include "render/texture_set.h"
#include "render/gl/gl_cbuffer.h"

struct GLFWwindow;
namespace op
{
    class RenderTexture;
    class Material;
    struct CBufferLayout;
    
    class GameResource final : public Singleton<GameResource>
    {
    public:
        Time time;
        Event<> onFrameEnd;
        Event<GLFWwindow*, int, int> onFrameBufferResize;

        GameResource();

        TextureSet* GetGlobalTextureSet() const { return m_globalTextureSet.get();}
        PerObjectBuffer* GetPerObjectBuffer() const { return m_perObjectBuffer.get();}
        GlCbuffer* GetPredefinedCbuffer(size_t nameId);
        bool IsPredefinedCbuffer(size_t nameId);
        bool NeedCreatePredefinedCbuffer(size_t nameId);
        void CreatePredefinedCbuffer(size_t nameId, crsp<CBufferLayout> layout);

        template <typename T>
        sp<T> GetResource(cr<StringHandle> path);
        sp<IResource> GetResource(cr<StringHandle> pathId);
        void RegisterResource(cr<StringHandle> pathId, crsp<IResource> resource);
        void UnregisterResource(cr<StringHandle> pathId);

    private:
        up<PerObjectBuffer> m_perObjectBuffer;
        umap<string_hash, wp<IResource>> m_resources;
        umap<string_hash, up<GlCbuffer>> m_predefinedCbuffers;
        up<TextureSet> m_globalTextureSet;
    };

    template <typename T>
    sp<T> GameResource::GetResource(cr<StringHandle> path)
    {
        auto result = GetResource(path);
        if (!result)
        {
            return nullptr;
        }

        auto result0 = std::dynamic_pointer_cast<T>(result);
        if (!result0)
        {
            THROW_ERRORF("Resource type miss match: %s", path.CStr());
        }

        return result0;
    }

    static GameResource* GetGR()
    {
        return GameResource::Ins();
    }

    static TextureSet* GetGlobalTextureSet()
    {
        return GetGR()->GetGlobalTextureSet();
    }

    static GlCbuffer* GetGlobalCbuffer()
    {
        return GetGR()->GetPredefinedCbuffer(GLOBAL_CBUFFER);
    }

    static GlCbuffer* GetPerViewCbuffer()
    {
        return GetGR()->GetPredefinedCbuffer(PER_VIEW_CBUFFER);
    }
}
