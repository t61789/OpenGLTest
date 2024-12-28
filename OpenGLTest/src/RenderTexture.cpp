#include "RenderTexture.h"

#include "Utils.h"

void checkGlFormatSupported(GLuint glFormat, RenderTextureFormat format)
{
    // GLuint support = ;
    // glGetInternalformativ ()
}

RenderTextureDescriptor::RenderTextureDescriptor() = default;

RenderTextureDescriptor::RenderTextureDescriptor(
    const size_t width,
    const size_t height,
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

void RenderTextureDescriptor::replaceSize(size_t width, size_t height)
{
    this->width = width;
    this->height = height;
}

RenderTexture::RenderTexture(const RenderTextureDescriptor& desc): Texture(0)
{
    recreate(desc);
}

RenderTexture::~RenderTexture()
{
    release();
}

void RenderTexture::recreate(const RenderTextureDescriptor& desc)
{
    if(isCreated)
    {
        glDeleteTextures(1, &glTextureId);
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
    Utils::CheckGlError("创建纹理");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    isCreated = true;
}

void RenderTexture::release()
{
    if(!isCreated)
    {
        return;
    }

    glDeleteTextures(1, &glTextureId);

    isCreated = false;
}

void RenderTexture::resize(size_t width, size_t height)
{
    desc.width = width;
    desc.height = height;

    release();
    recreate(desc);
}


