#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <tracy/Tracy.hpp>

#include "built_in_res.h"
#include "game_resource.h"
#include "utils.h"
#include "stb_image.h"
#include "common/asset_cache.h"
#include "render/gl/gl_state.h"
#include "render/gl/gl_texture.h"

namespace op
{
    Image::ImageCache Image::ImageCache::Create(const Image* image)
    {
        ImageCache cache;
        
        cache.width = image->m_width;
        cache.height = image->m_height;
        cache.data = image->m_data;
        cache.channels = image->m_channels;
        cache.format = image->m_glTexture->GetFormat();
        cache.type = image->m_glTexture->GetType();
        cache.importConfig = image->m_importConfig;

        return cache;
    }

    sp<Image> Image::LoadFromFile(cr<StringHandle> path)
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

        auto result = AssetCache::GetFromCache<Image, ImageCache>(path);
        
        GetGR()->RegisterResource(path, result);
        result->m_path = path;

        log_info("Load texture: %s", path.CStr());
        
        return result;
    }

    Image::ImportConfig Image::LoadImageImportConfig(crstr assetPath)
    {
        auto config = Utils::GetResourceMeta(assetPath);

        ImportConfig importConfig;

        if (config.contains("need_flip_vertical"))
        {
            importConfig.needFlipVertical = config.at("need_flip_vertical").get<bool>();
        }

        if (config.contains("need_mipmap"))
        {
            importConfig.needMipmap = config.at("need_mipmap").get<bool>();
        }
        
        if (config.contains("wrap_mode"))
        {
            importConfig.wrapMode = GlTexture::GetTextureWrapMode(config.at("wrap_mode").get<str>());
        }

        if (config.contains("filter_mode"))
        {
            importConfig.filterMode = GlTexture::GetTextureFilterMode(config.at("filter_mode").get<str>());
        }

        return importConfig;
    }

    sp<Image> Image::LoadFromFileImp(cr<StringHandle> path)
    {
        if (std::filesystem::is_directory(path.CStr()))
        {
            return LoadCubeFromFileImp(path);
        }

        if (!Utils::AssetExists(path))
        {
            return GetBR()->errorTex;
        }

        auto importConfig = LoadImageImportConfig(path);

        stbi_set_flip_vertically_on_load(importConfig.needFlipVertical);
        
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
        auto glTexture = GlTexture::Create2D(
            width,
            height,
            textureFormat,
            importConfig.wrapMode,
            importConfig.filterMode,
            data,
            importConfig.needMipmap);

        auto sizeB = width * height * nChannels;
        vec<uint8_t> dataVec(sizeB);
        memcpy(dataVec.data(), data, sizeB);

        stbi_image_free(data);

        auto result = msp<Image>();
        result->m_width = width;
        result->m_height = height;
        result->m_data = std::move(dataVec);
        result->m_channels = nChannels;
        result->m_glTexture = glTexture;
        result->m_importConfig = importConfig;

        return result;
    }

    sp<Image> Image::LoadCubeFromFileImp(cr<StringHandle> dirPath)
    {
        if (!Utils::AssetExists(dirPath))
        {
            THROW_ERRORF("Failed to load texture: %s", dirPath.CStr())
        }
        
        auto importConfig = LoadImageImportConfig(dirPath);

        stbi_set_flip_vertically_on_load(importConfig.needFlipVertical);
        
        int width = -1, height = -1, nChannels = -1;
        arr<uint8_t*, 6> cubeData = {};
        arr<cstr, 6> faces = {"right", "left", "top", "bottom", "front", "back"};
        try
        {
            for (uint32_t i = 0; i < 6; ++i)
            {
                auto path = dirPath.Str() + "/" + faces[i] + ".png";
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
        auto glTexture = GlTexture::CreateCube(
            width,
            height,
            textureFormat,
            importConfig.wrapMode,
            importConfig.filterMode,
            cubeData,
            importConfig.needMipmap);

        auto sizePerFaceB = width * height * nChannels;
        auto dataSizeB = sizePerFaceB * 6;
        auto data = vec<uint8_t>(dataSizeB);
        for (uint32_t i = 0; i < 6; ++i)
        {
            memcpy(data.data() + i * sizePerFaceB, cubeData[i], sizePerFaceB);
        }

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
        result->m_data = std::move(data);
        result->m_channels = nChannels;
        result->m_glTexture = glTexture;
        result->m_importConfig = importConfig;

        return result;
    }

    Image::ImageCache Image::CreateCacheFromAsset(crstr assetPath)
    {
        auto asset = LoadFromFileImp(assetPath);

        return ImageCache::Create(asset.get());
    }

    sp<Image> Image::CreateAssetFromCache(ImageCache&& cache)
    {
        auto c = std::move(cache);
        
        auto result = msp<Image>();
        result->m_width = c.width;
        result->m_height = c.height;

        if (c.type == GlTextureType::TEXTURE_2D)
        {
            result->m_glTexture = GlTexture::Create2D(
                c.width,
                c.height,
                c.format,
                c.importConfig.wrapMode,
                c.importConfig.filterMode,
                c.data.data(),
                c.importConfig.needMipmap);
        }
        else
        {
            std::array<uint8_t*, 6> data;
            for (uint32_t i = 0; i < 6; ++i)
            {
                data[i] = c.data.data() + i * c.width * c.height * c.channels;
            }
            
            result->m_glTexture = GlTexture::CreateCube(
                c.width,
                c.height,
                c.format,
                c.importConfig.wrapMode,
                c.importConfig.filterMode,
                data,
                c.importConfig.needMipmap);
        }

        return result;
    }
}
