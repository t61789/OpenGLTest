#include "Texture.h"

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLFormat =
{
    {RenderTextureFormat::RGBA, GL_RGBA},
    {RenderTextureFormat::RGB, GL_RGB},
    {RenderTextureFormat::RGBAHdr, GL_RGBA},
    {RenderTextureFormat::DepthStencil, GL_DEPTH_STENCIL},
    {RenderTextureFormat::Depth, GL_DEPTH_COMPONENT},
    {RenderTextureFormat::DepthTex, GL_RED}
};

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLInternalFormat =
{
    {RenderTextureFormat::RGBA, GL_RGBA},
    {RenderTextureFormat::RGB, GL_RGB},
    {RenderTextureFormat::RGBAHdr, GL_RGBA16F},
    {RenderTextureFormat::DepthStencil, GL_DEPTH24_STENCIL8 },
    {RenderTextureFormat::Depth, GL_DEPTH_COMPONENT},
    {RenderTextureFormat::DepthTex, GL_R32F}
};

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLType =
{
    {RenderTextureFormat::RGBA, GL_UNSIGNED_BYTE},
    {RenderTextureFormat::RGB, GL_UNSIGNED_BYTE},
    {RenderTextureFormat::RGBAHdr, GL_FLOAT},
    {RenderTextureFormat::DepthStencil, GL_UNSIGNED_INT_24_8},
    {RenderTextureFormat::Depth, GL_FLOAT},
    {RenderTextureFormat::DepthTex, GL_FLOAT}
};

std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLFilterMode =
{
    {TextureFilterMode::Point, GL_NEAREST},
    {TextureFilterMode::Bilinear, GL_LINEAR}
};

std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLMinFilterMode =
{
    {TextureFilterMode::Point, GL_NEAREST},
    {TextureFilterMode::Bilinear, GL_LINEAR_MIPMAP_LINEAR}
};

std::unordered_map<TextureWrapMode, GLuint> textureWrapModeToGLWrapMode =
{
    {TextureWrapMode::Repeat, GL_REPEAT},
    {TextureWrapMode::Clamp, GL_CLAMP_TO_EDGE},
    {TextureWrapMode::MirroredRepeat, GL_MIRRORED_REPEAT}
};

Texture::Texture(const GLuint glTextureId)
{
    this->width = 0;
    this->height = 0;
    this->glTextureId = glTextureId;
    this->isCreated = false;
}
