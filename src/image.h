#pragma once
#include "i_resource.h"
#include "i_texture.h"
#include "render/gl/gl_state.h"
#include "render/gl/gl_texture.h"

namespace op
{
    enum class TextureFilterMode : uint8_t;
    enum class TextureWrapMode : uint8_t;

    class Image final : public ITexture, public IResource
    {
        struct ImportConfig
        {
            bool needFlipVertical = true;
            bool needMipmap = true;
            TextureWrapMode wrapMode = TextureWrapMode::CLAMP;
            TextureFilterMode filterMode = TextureFilterMode::BILINEAR;

            template <typename Archive>
            void serialize(Archive& ar, uint32_t const version);
        };
        
        struct ImageCache
        {
            uint32_t width = 0;
            uint32_t height = 0;
            vec<uint8_t> data = {};
            uint32_t channels = 0;
            TextureFormat format = TextureFormat::RGBA;
            GlTextureType type = GlTextureType::TEXTURE_2D;
            ImportConfig importConfig;

            template <typename Archive>
            void serialize(Archive& ar, uint32_t const version);

            static ImageCache Create(const Image* image);
        };
        
    public:
        uint32_t GetWidth() override { return m_width;}
        uint32_t GetHeight() override { return m_height;}
        crsp<GlTexture> GetGlTexture() override { return m_glTexture;}
        cr<StringHandle> GetPath() override { return m_path;}

        static sp<Image> LoadFromFile(cr<StringHandle> path);

        static ImageCache CreateCacheFromAsset(crstr assetPath);
        static sp<Image> CreateAssetFromCache(ImageCache&& cache);

    private:
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        StringHandle m_path = {};
        
        sp<GlTexture> m_glTexture = nullptr;

        // serialization only fields
        uint32_t m_channels = 0;
        ImportConfig m_importConfig;
        vec<uint8_t> m_data;

        static ImportConfig LoadImageImportConfig(crstr assetPath);

        static sp<Image> LoadFromFileImp(cr<StringHandle> path);
        static sp<Image> LoadCubeFromFileImp(cr<StringHandle> dirPath);
    };

    template <typename Archive>
    void Image::ImportConfig::serialize(Archive& ar, uint32_t const version)
    {
        ar & needFlipVertical;
        ar & needMipmap;

        auto w = static_cast<uint8_t>(wrapMode);
        ar & w;
        wrapMode = static_cast<TextureWrapMode>(w);

        auto f = static_cast<uint8_t>(filterMode);
        ar & f;
        filterMode = static_cast<TextureFilterMode>(f);
    }

    template <typename Archive>
    void Image::ImageCache::serialize(Archive& ar, uint32_t const version)
    {
        ar & width;
        ar & height;
        ar & data;
        ar & channels;

        auto f = static_cast<uint8_t>(format);
        ar & f;
        format = static_cast<TextureFormat>(f);

        auto t = static_cast<uint8_t>(type);
        ar & t;
        type = static_cast<GlTextureType>(t);

        ar & importConfig;
    }
}
