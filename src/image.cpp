#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <tracy/Tracy.hpp>

#include "built_in_res.h"
#include "game_resource.h"
#include "utils.h"
#include "stb_image.h"
#include "render/gl/gl_texture.h"

namespace op
{
    cr<ImageDescriptor> ImageDescriptor::GetDefault()
    {
        constexpr static ImageDescriptor DEFAULT = {
            true,
            true,
            TextureWrapMode::CLAMP,
            TextureFilterMode::BILINEAR
        };
        return DEFAULT;
    }

    sp<Image> Image::LoadFromFile(cr<StringHandle> path, const ImageDescriptor& desc)
    {
        {
            if(auto result = GetGR()->GetResource<Image>(path))
            {
                return result;
            }
        }

        if (!Utils::AssetExists(path))
        {
            return GetBR()->errorTex;
        }

        stbi_set_flip_vertically_on_load(desc.needFlipVertical);
        
        int width = 0, height = 0, nChannels = 4;
        stbi_uc* data;
        try
        {
            ZoneScopedN("Load Image Data");
            
            data = stbi_load(Utils::GetAbsolutePath(path).c_str(), &width, &height, &nChannels, 0);
            if(!data)
            {
                throw std::exception();
            }
        }
        catch(cr<std::exception>)
        {
            log_info("Failed to load texture: %s", path.CStr());
            return GetBR()->errorTex;
        }

        auto textureFormat = nChannels == 4 ? TextureFormat::RGBA : TextureFormat::RGB;
        auto glTexture = GlTexture::Create2D(width, height, textureFormat, desc.wrapMode, desc.filterMode, data, desc.needMipmap);

        stbi_image_free(data);

        auto result = msp<Image>();
        result->m_width = width;
        result->m_height = height;
        result->m_glTexture = glTexture;
        
        GetGR()->RegisterResource(path, result);
        result->m_path = path;
        
        return result;
    }

    sp<Image> Image::LoadCubeFromFile(cr<StringHandle> dirPath, const std::string& expansionName, const ImageDescriptor& desc)
    {
        {
            if(auto result = GetGR()->GetResource<Image>(dirPath))
            {
                return result;
            }
        }
        
        if (!Utils::AssetExists(dirPath))
        {
            THROW_ERRORF("Failed to load texture: %s", dirPath.CStr())
        }

        stbi_set_flip_vertically_on_load(desc.needFlipVertical);
        
        int width = -1, height = -1, nChannels = -1;
        arr<uint8_t*, 6> cubeData = {};
        arr<cstr, 6> faces = {"right", "left", "top", "bottom", "front", "back"};
        try
        {
            for (uint32_t i = 0; i < 6; ++i)
            {
                auto path = dirPath.Str() + "/" + faces[i] + "." + expansionName;
                int curWidth = -1, curHeight = -1, curChannels = 4;
                
                auto data = stbi_load(Utils::GetAbsolutePath(path).c_str(), &curWidth, &curHeight, &curChannels, 0);
                if (!data || (width != -1 && curWidth != width) || (height != -1 && curHeight != height) || (nChannels != -1 && curChannels != nChannels))
                {
                    throw std::exception();
                }
                
                cubeData[i] = data;
                width = curWidth;
                height = curHeight;
                nChannels = curChannels;
            }
        }
        catch(cr<std::exception>)
        {
            THROW_ERRORF("Failed to load texture: %s", dirPath.CStr())
        }
        
        auto textureFormat = nChannels == 4 ? TextureFormat::RGBA : TextureFormat::RGB;
        auto glTexture = GlTexture::CreateCube(width, height, textureFormat, desc.wrapMode, desc.filterMode, cubeData, desc.needMipmap);

        for (uint32_t i = 0; i < 6; ++i)
        {
            if (cubeData[i])
            {
                stbi_image_free(cubeData[i]);
            }
        }
        
        auto result = msp<Image>();
        result->m_width = width;
        result->m_height = height;
        result->m_glTexture = glTexture;

        GetGR()->RegisterResource(dirPath, result);
        result->m_path = dirPath;
        
        return result;
    }
}
