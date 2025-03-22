#include "texture.h"

namespace op
{
    std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLFormat =
    {
        {RGBA, GL_RGBA},
        {RGB, GL_RGB},
        {RGBAHdr, GL_RGBA},
        {DepthStencil, GL_DEPTH_STENCIL},
        {Depth, GL_DEPTH_COMPONENT},
        {DepthTex, GL_RED}
    };

    std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLInternalFormat =
    {
        {RGBA, GL_RGBA},
        {RGB, GL_RGB},
        {RGBAHdr, GL_RGBA16F},
        {DepthStencil, GL_DEPTH24_STENCIL8 },
        {Depth, GL_DEPTH_COMPONENT},
        {DepthTex, GL_R32F}
    };

    std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLType =
    {
        {RGBA, GL_UNSIGNED_BYTE},
        {RGB, GL_UNSIGNED_BYTE},
        {RGBAHdr, GL_FLOAT},
        {DepthStencil, GL_UNSIGNED_INT_24_8},
        {Depth, GL_FLOAT},
        {DepthTex, GL_FLOAT}
    };

    std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLFilterMode =
    {
        {Point, GL_NEAREST},
        {Bilinear, GL_LINEAR}
    };

    std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLMinFilterMode =
    {
        {Point, GL_NEAREST},
        {Bilinear, GL_LINEAR_MIPMAP_LINEAR}
    };

    std::unordered_map<TextureWrapMode, GLuint> textureWrapModeToGLWrapMode =
    {
        {Repeat, GL_REPEAT},
        {Clamp, GL_CLAMP_TO_EDGE},
        {MirroredRepeat, GL_MIRRORED_REPEAT}
    };

    Texture::Texture(const GLuint glTextureId)
    {
        this->width = 0;
        this->height = 0;
        this->glTextureId = glTextureId;
        this->isCreated = false;
    }
}
