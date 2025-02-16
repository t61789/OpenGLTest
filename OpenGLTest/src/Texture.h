#pragma once

#include <glad/glad.h>

#include <string>
#include <stdexcept>

#include "SharedObject.h"

enum RenderTextureFormat
{
    RGBA,
    RGB,
    RGBAHdr,
    DepthStencil,
    Depth,
    DepthTex
};

enum TextureFilterMode
{
    Point,
    Bilinear,
};

enum TextureWrapMode
{
    Repeat,
    Clamp,
    MirroredRepeat,
};

extern std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLFormat;
extern std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLInternalFormat;
extern std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLType;
extern std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLFilterMode;
extern std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLMinFilterMode;
extern std::unordered_map<TextureWrapMode, GLuint> textureWrapModeToGLWrapMode;

class Texture : public SharedObject
{
public:
    bool isCreated;
    
    GLuint glTextureId;

    int width;
    int height;

    bool isCubeMap = false;

    Texture(GLuint glTextureId);
};
