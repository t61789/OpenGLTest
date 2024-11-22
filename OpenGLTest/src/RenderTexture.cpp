#include "RenderTexture.h"

RenderTexture::RenderTexture(
    const size_t width,
    const size_t height,
    const RenderTextureFormat format,
    const TextureFilterMode filterMode,
    const TextureWrapMode wrapMode)
{
    this->width = width;
    this->height = height;

    glGenTextures(1, &glTextureId);
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexImage2D(
        glTextureId,
        0,
        renderTextureFormatToGLFormat[format],
        static_cast<GLuint>(width),
        static_cast<GLuint>(height),
        0,
        renderTextureFormatToGLFormat[format],
        renderTextureFormatToGLType[format],
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapModeToGLWrapMode[wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapModeToGLWrapMode[wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterModeToGLFilterMode[filterMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterModeToGLFilterMode[filterMode]);

    glBindTexture(GL_TEXTURE_2D, 0);
}

RenderTexture::~RenderTexture()
{
    glDeleteTextures(1, &glTextureId);
}

