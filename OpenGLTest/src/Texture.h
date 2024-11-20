#pragma once
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <glad/glad.h>

#include "ResourceMgr.h"

class Texture
{
public:
    RESOURCE_ID m_id;
    GLuint m_glTextureId;
    
    static RESOURCE_ID LoadFromFile(const std::string& path);

private:
    
    Texture(GLuint glTextureId);
    ~Texture();

    // TODO 卸载纹理
};
