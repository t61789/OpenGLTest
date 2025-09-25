#include "gl_texture.h"

#include "gl_state.h"

namespace op
{
    GlTexture::GlTexture()
    {
        m_id = GlState::GlGenTexture();
        m_type = GlTextureType::TEXTURE_2D;
    }

    GlTexture::~GlTexture()
    {
        GlState::Ins()->UnRegisterTexture(this);
        GlState::GlDeleteTexture(m_id);
    }

    void GlTexture::Bind(const uint32_t slot)
    {
        GlState::Ins()->BindTexture(slot, this);
    }

    sp<GlTexture> GlTexture::Create2D(
        const uint32_t width,
        const uint32_t height,
        const TextureFormat format,
        const TextureWrapMode wrapMode,
        const TextureFilterMode filterMode,
        const uint8_t* data,
        const bool needMipmap)
    {
        auto texture = CreateBasicTexture(
            GlTextureType::TEXTURE_2D,
            width,
            height,
            format,
            wrapMode,
            filterMode);

        GlState::GlTexImage2D(
            texture->GetType(),
            0,
            GetGlInternalFormat(format),
            width,
            height,
            0,
            GetGlFormat(format),
            GetGlStoreType(format),
            data);

        texture->m_hasMipmap = needMipmap && data;
        if (texture->m_hasMipmap)
        {
            GlState::GlGenerateMipmap(texture->GetType());
        }

        return texture;
    }

    sp<GlTexture> GlTexture::CreateCube(
        const uint32_t width,
        const uint32_t height,
        const TextureFormat format,
        const TextureWrapMode wrapMode,
        const TextureFilterMode filterMode,
        const arr<uint8_t*, 6>& data,
        const bool needMipmap)
    {
        auto texture = CreateBasicTexture(
            GlTextureType::TEXTURE_CUBE_MAP,
            width,
            height,
            format,
            wrapMode,
            filterMode);

        for (uint32_t i = 0; i < data.size(); i++)
        {
            GlState::GlTexImageCube(
                i,
                0,
                GetGlInternalFormat(format),
                width,
                height,
                0,
                GetGlFormat(format),
                GetGlStoreType(format),
                data[i]);
        }
        
        texture->m_hasMipmap = needMipmap;
        if (texture->m_hasMipmap)
        {
            GlState::GlGenerateMipmap(texture->GetType());
        }

        return texture;
    }

    TextureWrapMode GlTexture::GetTextureWrapMode(cr<StringHandle> s)
    {
        const static umap<string_hash, TextureWrapMode> MAPPER = {
            {StringHandle("Clamp"), TextureWrapMode::CLAMP},
            {StringHandle("Repeat"), TextureWrapMode::REPEAT},
            {StringHandle("MirroredRepeat"), TextureWrapMode::MIRRORED_REPEAT},
        };

        return MAPPER.at(s);
    }

    TextureFilterMode GlTexture::GetTextureFilterMode(cr<StringHandle> s)
    {
        const static umap<string_hash, TextureFilterMode> MAPPER = {
            {StringHandle("Point"), TextureFilterMode::POINT},
            {StringHandle("Bilinear"), TextureFilterMode::BILINEAR},
        };

        return MAPPER.at(s);
    }

    sp<GlTexture> GlTexture::CreateBasicTexture(
        const GlTextureType type,
        const uint32_t width,
        const uint32_t height,
        const TextureFormat format,
        const TextureWrapMode wrapMode,
        const TextureFilterMode filterMode)
    {
        auto texture = msp<GlTexture>();
        texture->m_type = type;
        texture->m_width = width;
        texture->m_height = height;
        texture->m_format = format;
        texture->m_wrapMode = wrapMode;
        texture->m_filterMode = filterMode;

        GlState::Ins()->BindTexture(0, texture.get());
        GlState::GlPixelStore(GL_UNPACK_ALIGNMENT, 1);
        GlState::GlTexParameter(texture->GetType(), GL_TEXTURE_WRAP_S, GetGlWrapMode(wrapMode));
        GlState::GlTexParameter(texture->GetType(), GL_TEXTURE_WRAP_T, GetGlWrapMode(wrapMode));
        GlState::GlTexParameter(texture->GetType(), GL_TEXTURE_MIN_FILTER, GetGlMinFilter(filterMode));
        GlState::GlTexParameter(texture->GetType(), GL_TEXTURE_MAG_FILTER, GetGlMagFilter(filterMode));

        return texture;
    }

    GLenum GlTexture::GetGlInternalFormat(const TextureFormat format)
    {
        const static umap<TextureFormat, GLenum> MAPPER = {
            {TextureFormat::RGBA, GL_RGBA},
            {TextureFormat::RGB, GL_RGB},
            {TextureFormat::RGBA_HDR, GL_RGBA16F},
            {TextureFormat::DEPTH_STENCIL, GL_DEPTH24_STENCIL8},
            {TextureFormat::DEPTH, GL_DEPTH_COMPONENT},
            {TextureFormat::DEPTH_TEX, GL_R32F},
        };

        return MAPPER.at(format);
    }

    GLenum GlTexture::GetGlFormat(const TextureFormat format)
    {
        const static umap<TextureFormat, GLenum> MAPPER = {
            {TextureFormat::RGBA, GL_RGBA},
            {TextureFormat::RGB, GL_RGB},
            {TextureFormat::RGBA_HDR, GL_RGBA},
            {TextureFormat::DEPTH_STENCIL, GL_DEPTH_STENCIL},
            {TextureFormat::DEPTH, GL_DEPTH_COMPONENT},
            {TextureFormat::DEPTH_TEX, GL_RED},
        };

        return MAPPER.at(format);
    }

    GLenum GlTexture::GetGlStoreType(const TextureFormat format)
    {
        const static umap<TextureFormat, GLenum> MAPPER = {
            {TextureFormat::RGBA, GL_UNSIGNED_BYTE},
            {TextureFormat::RGB, GL_UNSIGNED_BYTE},
            {TextureFormat::RGBA_HDR, GL_FLOAT},
            {TextureFormat::DEPTH_STENCIL, GL_UNSIGNED_INT_24_8},
            {TextureFormat::DEPTH, GL_FLOAT},
            {TextureFormat::DEPTH_TEX, GL_FLOAT}
        };

        return MAPPER.at(format);
    }

    GLenum GlTexture::GetGlMinFilter(const TextureFilterMode filterMode)
    {
        const static umap<TextureFilterMode, GLenum> MAPPER = {
            {TextureFilterMode::POINT, GL_NEAREST},
            {TextureFilterMode::BILINEAR, GL_LINEAR},
        };

        return MAPPER.at(filterMode);
    }

    GLenum GlTexture::GetGlMagFilter(const TextureFilterMode filterMode)
    {
        const static umap<TextureFilterMode, GLenum> MAPPER = {
            {TextureFilterMode::POINT, GL_NEAREST},
            {TextureFilterMode::BILINEAR, GL_LINEAR},
        };

        return MAPPER.at(filterMode);
    }

    GLenum GlTexture::GetGlWrapMode(const TextureWrapMode wrapMode)
    {
        const static umap<TextureWrapMode, GLenum> MAPPER = {
            {TextureWrapMode::CLAMP, GL_CLAMP_TO_EDGE},
            {TextureWrapMode::REPEAT, GL_REPEAT},
            {TextureWrapMode::MIRRORED_REPEAT, GL_MIRRORED_REPEAT}
        };

        return MAPPER.at(wrapMode);
    }
}
