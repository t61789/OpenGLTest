#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Utils.h"
#include "stb_image.h"

ImageDescriptor ImageDescriptor::GetDefault()
{
    ImageDescriptor result;
    result.filterMode = Bilinear;
    result.wrapMode = Clamp;
    result.needFlipVertical = true;
    result.needMipmap = true;
    return result;
}

Image::Image(const GLuint glTextureId) : Texture(glTextureId)
{
    
}

Image::~Image()
{
    if(isCreated)
    {
        glDeleteTextures(1, &glTextureId);
    }
}

Image* Image::LoadFromFile(const std::string& path, const ImageDescriptor& desc)
{
    {
        SharedObject* result;
        if(TryGetResource(path, result))
        {
            return dynamic_cast<Image*>(result);
        }
    }

    stbi_set_flip_vertically_on_load(desc.needFlipVertical);
    
    int width, height, nChannels;
    stbi_uc* data;
    try
    {
        data = stbi_load(Utils::GetAbsolutePath(path).c_str(), &width, &height, &nChannels, 0);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO may cause performance problem
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    if(desc.needMipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(data);

    auto result = new Image(glTextureId);
    result->width = width;
    result->height = height;
    result->isCreated = true;
    RegisterResource(path, result);
    return result;
}

Image* Image::LoadCubeFromFile(const std::string& dirPath, const std::string& expansionName, const ImageDescriptor& desc)
{
    {
        SharedObject* result;
        if(TryGetResource(dirPath, result))
        {
            return dynamic_cast<Image*>(result);
        }
    }

    stbi_set_flip_vertically_on_load(desc.needFlipVertical);
    
    GLuint glTextureId;
    glGenTextures(1, &glTextureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, glTextureId);
    int width = 0, height = 0, nChannels;
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, textureWrapModeToGLWrapMode[desc.wrapMode]);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, textureFilterModeToGLMinFilterMode[desc.filterMode]);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, textureFilterModeToGLFilterMode[desc.filterMode]);
    
    std::string faces[6]= {"right", "left", "top", "bottom", "front", "back"};
    try
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // TODO may cause performance problem
        for (int i = 0; i < std::size(faces); ++i)
        {
            auto path = dirPath + "/" + faces[i] + "." + expansionName;
            auto data = stbi_load(Utils::GetAbsolutePath(path).c_str(), &width, &height, &nChannels, 0);
            if(!data)
            {
                stbi_image_free(data);
                throw std::runtime_error("ERROR>> Failed to load texture: " + std::string(path));
            }
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }
    catch(std::exception)
    {
        glDeleteTextures(1, &glTextureId);
        throw;
    }

    if(desc.needMipmap)
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    
    auto result = new Image(glTextureId);
    result->width = width;
    result->height = height;
    result->isCreated = true;
    result->isCubeMap = true;
    RegisterResource(dirPath, result);
    return result;
}
