#pragma once
#include "Texture.h"

class Image : public Texture
{
public:
    
    ~Image() override;
    
    static RESOURCE_ID LoadFromFile(const std::string& path);
};
