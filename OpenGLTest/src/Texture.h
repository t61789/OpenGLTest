#pragma once
#include <string>
#include <stdexcept>
#include <glad/glad.h>

class Texture
{
public:
    GLuint textureId;
    
    Texture(const std::string& path);
};
