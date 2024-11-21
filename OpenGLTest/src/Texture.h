#pragma once

#include <glad/glad.h>

#include <string>
#include <stdexcept>

#include "ResourceMgr.h"

class Texture : ResourceBase
{
public:
    GLuint glTextureId;
    
    static RESOURCE_ID LoadFromFile(const std::string& path);

private:
    
    Texture(GLuint glTextureId);
    ~Texture() override;
};
