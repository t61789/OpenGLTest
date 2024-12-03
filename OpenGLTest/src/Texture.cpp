#include "Texture.h"

Texture::Texture(const GLuint glTextureId)
{
    this->width = 0;
    this->height = 0;
    this->glTextureId = glTextureId;
    this->isCreated = false;
}
