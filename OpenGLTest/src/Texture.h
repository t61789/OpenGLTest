#pragma once

#include <glad/glad.h>

#include <string>
#include <stdexcept>

#include "ResourceMgr.h"

class Texture : public ResourceBase
{
public:
    bool isCreated;
    
    GLuint glTextureId;

    size_t width;
    size_t height;

    bool isCubeMap = false;

    Texture(GLuint glTextureId);
};
