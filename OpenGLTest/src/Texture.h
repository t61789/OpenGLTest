#pragma once
#include <string>
#include <stdexcept>
#include <glad/glad.h>

#include "ResourceMgr.h"

class Texture : ResourceBase
{
public:
    GLuint m_glTextureId;
    
    static RESOURCE_ID LoadFromFile(const std::string& path);

private:
    
    Texture(GLuint glTextureId);
    ~Texture() override;
};
