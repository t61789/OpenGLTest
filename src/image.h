#pragma once
#include "i_resource.h"
#include "i_texture.h"

namespace op
{
    enum class TextureFilterMode : uint8_t;
    enum class TextureWrapMode : uint8_t;

    struct ImageDescriptor
    {
        bool needFlipVertical;
        bool needMipmap;
        TextureWrapMode wrapMode;
        TextureFilterMode filterMode;

        static cr<ImageDescriptor> GetDefault();
    };

    class Image final : public ITexture, public IResource
    {
    public:
        uint32_t GetWidth() override { return m_width;}
        uint32_t GetHeight() override { return m_height;}
        crsp<GlTexture> GetGlTexture() override { return m_glTexture;}
        cr<StringHandle> GetPath() override { return m_path;}

        static sp<Image> LoadFromFile(cr<StringHandle> path, const ImageDescriptor& desc);
        static sp<Image> LoadCubeFromFile(cr<StringHandle> dirPath, const std::string& expansionName, const ImageDescriptor& desc);

    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        StringHandle m_path = {};
        
        sp<GlTexture> m_glTexture = nullptr;
    };
}
