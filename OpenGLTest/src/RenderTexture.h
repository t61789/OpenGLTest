#pragma once
#include <unordered_map>
#include <glad/glad.h>

#include "ResourceMgr.h"
#include "Texture.h"

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
    RenderTextureDescriptor(size_t width, size_t height, RenderTextureFormat format, TextureFilterMode filterMode, TextureWrapMode wrapMode,
                            std::string name = "Unnamed RenderTexture");
    void replaceSize(size_t width, size_t height);
};

class RenderTexture : public Texture
{
public:
    
    RenderTextureDescriptor desc;

    RenderTexture(const RenderTextureDescriptor& desc);
    ~RenderTexture() override;

    void recreate(const RenderTextureDescriptor& desc);
    void release();
    void resize(size_t width, size_t height);
};