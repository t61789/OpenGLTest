#pragma once

#include "glad.h"

#include "shared_object.h"

namespace op
{
    enum RenderTextureFormat : std::uint8_t
    {
        RGBA,
        RGB,
        RGBAHdr,
        DepthStencil,
        Depth,
        DepthTex
    };

    enum TextureFilterMode : std::uint8_t
    {
        Point,
        Bilinear,
    };

    enum TextureWrapMode : std::uint8_t
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

        int width = 0;
        int height = 0;

        bool isCubeMap = false;

        Texture(GLuint glTextureId);
    };
}
