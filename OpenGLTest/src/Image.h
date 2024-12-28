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

    ~Image() override;
    
    static RESOURCE_ID LoadFromFile(const std::string& path, const ImageDescriptor& desc);
    static RESOURCE_ID LoadCubeFromFile(const std::string& dirPath, const std::string& expansionName, const ImageDescriptor& desc);
};
