#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Utils.h"
#include "../lib/stb_image.h"

Image::~Image()
{
    if(isCreated)
    {
        glDeleteTextures(1, &glTextureId);
    }
}

RESOURCE_ID Image::LoadFromFile(const std::string& path)
{
    if(ResourceMgr::IsResourceRegistered(path))
    {
        return ResourceMgr::GetRegisteredResource(path);
    }

    stbi_set_flip_vertically_on_load(true);
    
    int width, height, nChannels;
    stbi_uc* data;
    try
    {
        data = stbi_load(Utils::GetRealAssetPath(path).c_str(), &width, &height, &nChannels, 0);
        if(!data)
        {
            throw std::runtime_error("ERROR>> Failed to load texture: " + std::string(path));
        }
    }
    catch(std::exception)
    {
        throw;
    }

    GLuint glTextureId;
    glGenTextures(1, &glTextureId);
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO may cause performance problem
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    auto texture = new Texture(glTextureId);
    texture->width = width;
    texture->height = height;
    texture->isCreated = true;
    ResourceMgr::RegisterResource(path, texture->id);
    return texture->id;
}
