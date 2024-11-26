#include "RenderTexture.h"

#include "Utils.h"

extern std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLFormat =
{
    {RenderTextureFormat::RGBA, GL_RGBA},
    {RenderTextureFormat::RGB, GL_RGB},
    {RenderTextureFormat::RGBAHdr, GL_RGBA},
    {RenderTextureFormat::DepthStencil, GL_DEPTH24_STENCIL8},
    {RenderTextureFormat::Depth, GL_DEPTH_COMPONENT}
};

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLInternalFormat =
{
    {RenderTextureFormat::RGBA, GL_RGBA},
    {RenderTextureFormat::RGB, GL_RGB},
    {RenderTextureFormat::RGBAHdr, GL_RGBA},
    {RenderTextureFormat::DepthStencil, GL_DEPTH24_STENCIL8},
    {RenderTextureFormat::Depth, GL_DEPTH_COMPONENT}
};

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLType =
{
    {RenderTextureFormat::RGBA, GL_UNSIGNED_BYTE},
    {RenderTextureFormat::RGB, GL_UNSIGNED_BYTE},
    {RenderTextureFormat::RGBAHdr, GL_UNSIGNED_SHORT},
    {RenderTextureFormat::DepthStencil, GL_UNSIGNED_INT_24_8},
    {RenderTextureFormat::Depth, GL_FLOAT}
};

std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLFilterMode =
{
    {TextureFilterMode::Point, GL_NEAREST},
    {TextureFilterMode::Bilinear, GL_LINEAR}
};

std::unordered_map<TextureWrapMode, GLuint> textureWrapModeToGLWrapMode =
{
    {TextureWrapMode::Repeat, GL_REPEAT},
    {TextureWrapMode::Clamp, GL_CLAMP_TO_EDGE},
    {TextureWrapMode::MirroredRepeat, GL_MIRRORED_REPEAT}
};

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
    const TextureWrapMode wrapMode)
    : width(width),
      height(height),
      format(format),
      filterMode(filterMode),
      wrapMode(wrapMode)
{
}

RenderTexture::RenderTexture(const RenderTextureDescriptor& desc)
{
    this->desc = desc;

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

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        Utils::Log("创建纹理失败：" + std::to_string(error), Error);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);

    glBindTexture(GL_TEXTURE_2D, 0);
}

RenderTexture::~RenderTexture()
{
    glDeleteTextures(1, &glTextureId);
}

