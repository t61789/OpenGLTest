#include "render_texture.h"

#include "texture.h"
#include "utils.h"

namespace op
{
    static void CheckGlFormatSupported(GLuint glFormat, RenderTextureFormat format)
    {
        // GLuint support = ;
        // glGetInternalformativ ()
    }

    RenderTextureDescriptor::RenderTextureDescriptor() = default;

    RenderTextureDescriptor::RenderTextureDescriptor(
        const int width,
        const int height,
        const RenderTextureFormat format,
        const TextureFilterMode filterMode,
        const TextureWrapMode wrapMode,
        std::string name):
        name(std::move(name)),
        width(width),
        height(height),
        format(format),
        filterMode(filterMode),
        wrapMode(wrapMode)
    {
        
    }

    void RenderTextureDescriptor::replaceSize(int width, int height)
    {
        this->width = width;
        this->height = height;
    }

    RenderTexture::RenderTexture(const RenderTextureDescriptor& desc): Texture(0)
    {
        Init(desc);
    }

    RenderTexture::RenderTexture(
        const int width,
        const int height,
        const RenderTextureFormat format,
        const TextureFilterMode filterMode,
        const TextureWrapMode wrapMode,
        const std::string& name) : Texture(0)
    {
        Init(RenderTextureDescriptor(
            width,
            height,
            format,
            filterMode,
            wrapMode,
            name));
    }

    RenderTexture::~RenderTexture()
    {
        Release();
    }

    void RenderTexture::Recreate(const RenderTextureDescriptor& desc)
    {
        if(isCreated)
        {
            Release();
        }
        
        this->desc = desc;
        width = desc.width;
        height = desc.height;

        auto glInternalFormat = renderTextureFormatToGLInternalFormat[desc.format];
        auto glFormat = renderTextureFormatToGLFormat[desc.format];
        auto glType = renderTextureFormatToGLType[desc.format];

        glGenTextures(1, &glTextureId);
        glBindTexture(GL_TEXTURE_2D, glTextureId);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            glInternalFormat,
            static_cast<GLuint>(desc.width),
            static_cast<GLuint>(desc.height),
            0,
            glFormat,
            glType,
            0);
        GL_CHECK_ERROR(创建纹理)

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapModeToGLWrapMode[desc.wrapMode]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapModeToGLWrapMode[desc.wrapMode]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
        
        glBindTexture(GL_TEXTURE_2D, 0);

        isCreated = true;
    }

    void RenderTexture::Release()
    {
        if(!isCreated)
        {
            return;
        }

        glDeleteTextures(1, &glTextureId);

        isCreated = false;
    }

    void RenderTexture::Resize(int width, int height)
    {
        desc.width = width;
        desc.height = height;

        if (this->width == width && this->height == height)
        {
            return;
        }

        Recreate(desc);

        onResize->Invoke();
    }

    void RenderTexture::Init(const RenderTextureDescriptor& desc)
    {
        onResize = std::make_unique<Event<>>();
        
        Recreate(desc);
    }
}
