#pragma once
#include <unordered_map>
#include <glad/glad.h>

#include "ResourceMgr.h"

enum RenderTextureFormat
{
    RGBA,
    RGB,
    RGBAHdr,
    DepthStencil,
    Depth
};

enum TextureFilterMode
{
    Point,
    Bilinear,
};

enum TextureWrapMode
{
    Repeat,
    Clamp,
    MirroredRepeat,
};

class RenderTexture : public ResourceBase
{
public:
    GLuint glTextureId;
    
    size_t width;
    size_t height;

    RenderTexture(
        size_t width,
        size_t height,
        RenderTextureFormat format,
        TextureFilterMode filterMode,
        TextureWrapMode wrapMode);
    ~RenderTexture();
};