#pragma once

#include "utils.h"

namespace op
{
    enum class GlTextureType : uint8_t;
    
    enum class TextureFormat : uint8_t
    {
        RGBA,
        RGB,
        RGBA_HDR,
        DEPTH_STENCIL,
        DEPTH,
        DEPTH_TEX
    };

    enum class TextureFilterMode : uint8_t
    {
        POINT,
        BILINEAR,
    };

    enum class TextureWrapMode : uint8_t
    {
        REPEAT,
        CLAMP,
        MIRRORED_REPEAT,
    };
    
    class GlTexture final : public std::enable_shared_from_this<GlTexture>
    {
    public:
        GlTexture();
        ~GlTexture();
        GlTexture(const GlTexture& other) = delete;
        GlTexture(GlTexture&& other) noexcept = delete;
        GlTexture& operator=(const GlTexture& other) = delete;
        GlTexture& operator=(GlTexture&& other) noexcept = delete;
        
        uint32_t GetId() const { return m_id;}
        TextureFormat GetFormat() const { return m_format;}
        GlTextureType GetType() const { return m_type;}
        uint32_t GetWidth() const { return m_width;}
        uint32_t GetHeight() const { return m_height;}
        bool HasMipmap() const { return m_hasMipmap;}

        void Bind(uint32_t slot);

        static sp<GlTexture> Create2D(
            uint32_t width,
            uint32_t height,
            TextureFormat format,
            TextureWrapMode wrapMode = TextureWrapMode::CLAMP,
            TextureFilterMode filterMode = TextureFilterMode::BILINEAR,
            const uint8_t* data = nullptr,
            bool needMipmap = true);
        
        static sp<GlTexture> CreateCube(
            uint32_t width,
            uint32_t height,
            TextureFormat format,
            TextureWrapMode wrapMode = TextureWrapMode::CLAMP,
            TextureFilterMode filterMode = TextureFilterMode::BILINEAR,
            cr<arr<uint8_t*, 6>> data = {},
            bool needMipmap = true);

        static TextureWrapMode GetTextureWrapMode(cr<StringHandle> s);
        static TextureFilterMode GetTextureFilterMode(cr<StringHandle> s);

    private:
        uint32_t m_id = 0;
        GlTextureType m_type;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        TextureFormat m_format = TextureFormat::RGBA;
        TextureWrapMode m_wrapMode = TextureWrapMode::CLAMP;
        TextureFilterMode m_filterMode = TextureFilterMode::BILINEAR;
        bool m_hasMipmap = false;

        static sp<GlTexture> CreateBasicTexture(
            GlTextureType type,
            uint32_t width,
            uint32_t height,
            TextureFormat format,
            TextureWrapMode wrapMode,
            TextureFilterMode filterMode);

        static GLenum GetGlInternalFormat(TextureFormat format);
        static GLenum GetGlFormat(TextureFormat format);
        static GLenum GetGlStoreType(TextureFormat format);
        static GLenum GetGlMinFilter(TextureFilterMode filterMode);
        static GLenum GetGlMagFilter(TextureFilterMode filterMode);
        static GLenum GetGlWrapMode(TextureWrapMode wrapMode);
    };
}
