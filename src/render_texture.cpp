#include "render_texture.h"

#include "utils.h"
#include "render/gl/gl_texture.h"

namespace op
{
    RenderTexture::RenderTexture(const RtDesc& desc)
    {
        Init(desc);
    }

    RenderTexture::~RenderTexture()
    {
        Release();
    }

    void RenderTexture::Recreate(const RtDesc& desc)
    {
        Release();
        
        m_desc = desc;
        m_glTexture = GlTexture::Create2D(
            desc.width,
            desc.height,
            desc.format,
            desc.wrapMode,
            desc.filterMode,
            nullptr,
            false);
    }

    void RenderTexture::Release()
    {
        m_glTexture.reset();
    }

    void RenderTexture::Resize(const uint32_t width, const uint32_t height)
    {
        if (GetWidth() == width && GetHeight() == height)
        {
            return;
        }
        
        m_desc.width = width;
        m_desc.height = height;

        Recreate(m_desc);

        onResize.Invoke();
    }

    void RenderTexture::Init(const RtDesc& desc)
    {
        Recreate(desc);
    }
}
