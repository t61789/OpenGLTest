#pragma once
#include <unordered_map>
#include <glad/glad.h>

#include "SharedObject.h"
#include "Texture.h"

class RenderTextureDescriptor
{
public:
    std::string name = "Unnamed Render Texture";
    int width;
    int height;
    RenderTextureFormat format;
    TextureFilterMode filterMode;
    TextureWrapMode wrapMode;

    RenderTextureDescriptor();
    RenderTextureDescriptor(int width, int height, RenderTextureFormat format, TextureFilterMode filterMode, TextureWrapMode wrapMode,
                            std::string name = "Unnamed RenderTexture");
    void replaceSize(int width, int height);
};

class RenderTexture : public Texture
{
public:
    
    RenderTextureDescriptor desc;

    RenderTexture(const RenderTextureDescriptor& desc);
    ~RenderTexture() override;

    void Recreate(const RenderTextureDescriptor& desc);
    void Release();
    void Resize(int width, int height);
};