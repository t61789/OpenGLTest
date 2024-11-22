#pragma once
#include <unordered_map>
#include <glad/glad.h>

#include "ResourceMgr.h"

enum RenderTextureFormat
{
   RGBA,
   RGB,
   DepthStencil,
   Depth
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

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLFormat =
{
   {RenderTextureFormat::RGBA, GL_RGBA},
   {RenderTextureFormat::RGB, GL_RGB},
   {RenderTextureFormat::DepthStencil, GL_DEPTH24_STENCIL8},
   {RenderTextureFormat::Depth, GL_DEPTH_COMPONENT}
};

std::unordered_map<RenderTextureFormat, GLuint> renderTextureFormatToGLType =
{
   {RenderTextureFormat::RGBA, GL_UNSIGNED_BYTE},
   {RenderTextureFormat::RGB, GL_UNSIGNED_BYTE},
   {RenderTextureFormat::DepthStencil, GL_UNSIGNED_INT_24_8},
   {RenderTextureFormat::Depth, GL_FLOAT}
};

std::unordered_map<TextureFilterMode, GLuint> textureFilterModeToGLFilterMode =
{
   {TextureFilterMode::Point, GL_NEAREST},
   {TextureFilterMode::Bilinear, GL_LINEAR}
};

std::unordered_map<TextureWrapMode, GLuint> textureWrapModeToGLWrapMode =
{
   {TextureWrapMode::Repeat, GL_REPEAT},
   {TextureWrapMode::Clamp, GL_CLAMP_TO_EDGE},
   {TextureWrapMode::MirroredRepeat, GL_MIRRORED_REPEAT}
};

class RenderTexture : public ResourceBase
{
public:
   GLuint glTextureId;
   
   size_t width;
   size_t height;

   RenderTexture(
      size_t width,
      size_t height,
      RenderTextureFormat format,
      TextureFilterMode filterMode,
      TextureWrapMode wrapMode);
   ~RenderTexture();
};
