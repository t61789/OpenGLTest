#pragma once

#include <memory>

#include "event.h"
#include "i_texture.h"
#include "shared_object.h"

namespace op
{
    enum class TextureWrapMode : std::uint8_t;
    enum class TextureFilterMode : std::uint8_t;
    enum class TextureFormat : std::uint8_t;

    struct RtDesc
    {
        std::string name;
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        TextureFilterMode filterMode;
        TextureWrapMode wrapMode;
    };

    class RenderTexture final : public ITexture
    {
    public:
        explicit RenderTexture(const RtDesc& desc);
        ~RenderTexture() override;
        RenderTexture(const RenderTexture& other) = delete;
        RenderTexture(RenderTexture&& other) noexcept = delete;
        RenderTexture& operator=(const RenderTexture& other) = delete;
        RenderTexture& operator=(RenderTexture&& other) noexcept = delete;

        void Recreate(const RtDesc& desc);
        void Release();
        void Resize(uint32_t width, uint32_t height);

        Event<> onResize;

        RtDesc GetDescriptor() const { return m_desc;}
        uint32_t GetWidth() override { return m_desc.width;}
        uint32_t GetHeight() override { return m_desc.height;}
        crsp<GlTexture> GetGlTexture() override { return m_glTexture;}

    private:
        RtDesc m_desc;
        
        sp<GlTexture> m_glTexture = nullptr;
        
        void Init(const RtDesc& desc);
    };
}
