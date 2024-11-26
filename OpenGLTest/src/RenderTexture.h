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
    Depth,
    DepthTex
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

class RenderTextureDescriptor
{
public:
    std::string name = "Unnamed Render Texture";
    size_t width;
    size_t height;
    RenderTextureFormat format;
    TextureFilterMode filterMode;
    TextureWrapMode wrapMode;

    RenderTextureDescriptor();
    RenderTextureDescriptor(size_t width, size_t height, RenderTextureFormat format, TextureFilterMode filterMode, TextureWrapMode wrapMode);
};

class RenderTexture : public ResourceBase
{
public:
    GLuint glTextureId;
    
    RenderTextureDescriptor desc;

    RenderTexture(const RenderTextureDescriptor& desc);
    ~RenderTexture();
};