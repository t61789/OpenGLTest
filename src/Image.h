#pragma once
#include "Texture.h"

class ImageDescriptor
{
public:
    bool needFlipVertical;
    bool needMipmap;
    TextureWrapMode wrapMode;
    TextureFilterMode filterMode;

    static ImageDescriptor GetDefault();
};

class Image : public Texture
{
public:
    Image(GLuint glTextureId);
    ~Image() override;
    
    static Image* LoadFromFile(const std::string& path, const ImageDescriptor& desc);
    static Image* LoadCubeFromFile(const std::string& dirPath, const std::string& expansionName, const ImageDescriptor& desc);
};
