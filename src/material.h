#pragma once

#include "i_resource.h"
#include "common/data_set.h"
#include "math/matrix4x4.h"
#include "math/vec.h"
#include "render/texture_set.h"
#include "render/gl/gl_cbuffer.h"

namespace op
{
    struct CBufferLayout;
    class Shader;
    class ITexture;
    enum class BlendMode : uint8_t;
    enum class CullMode : uint8_t;

    class Material final : public IResource
    {
    public:
        CullMode cullMode;
        BlendMode blendMode;
        
        Material();

        template <typename T>
        void Set(size_t nameId, cr<T> val);
        void Set(size_t nameId, crsp<ITexture> val);
        void Set(size_t nameId, const float* val, size_t count);

        template <typename T>
        T Get(size_t nameId);
        void Get(size_t nameId, float* val, size_t count);
        sp<ITexture> GetTexture(size_t nameId);

        cr<StringHandle> GetPath() override { return m_path;}
        sp<Shader> GetShader() const { return m_shader; }
        bool HasCBuffer() { return m_cbuffer != nullptr; }
        TextureSet* GetTextureSet() { return m_textureSet.get(); }
        
        void BindShader(crsp<Shader> shader);
        void CreateCBuffer(crsp<CBufferLayout> cbufferLayout);
        void UseCBuffer();

        static sp<Material> LoadFromFile(cr<StringHandle> path);
        static sp<Material> CreateFromShader(cr<StringHandle> path);
        
    private:
        struct ValueInfo
        {
            size_t nameId;
            uint32_t byteCount;
            uint8_t* data = nullptr;
        };

        StringHandle m_path;

        sp<Shader> m_shader = nullptr;
        up<GlCbuffer> m_cbuffer = nullptr;
        up<DataSet> m_dataSet = nullptr;
        up<TextureSet> m_textureSet = nullptr;

        void OnFrameEnd();

        bool TryGetImp(string_hash nameId, void* data, uint32_t sizeB);
        bool TrySetImp(string_hash nameId, const void* data, uint32_t sizeB);
    };

    template <typename T>
    void Material::Set(const size_t nameId, cr<T> val)
    {
        TrySetImp(nameId, &val, sizeof(T));
    }

    template <typename T>
    T Material::Get(const size_t nameId)
    {
        T val;
        TryGetImp(nameId, &val, sizeof(T));
        return val;
    }
}
