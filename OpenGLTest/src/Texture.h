#pragma once

#include <glad/glad.h>

#include <string>
#include <stdexcept>

#include "ResourceMgr.h"

class Texture : public ResourceBase
{
public:
    GLuint glTextureId;

    size_t width;
    size_t height;

    Texture(GLuint glTextureId);
};
