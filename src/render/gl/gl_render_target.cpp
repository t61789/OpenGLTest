#include "gl_render_target.h"

#include "game_framework.h"
#include "game_resource.h"
#include "gl_state.h"
#include "gl_texture.h"

namespace op
{
    GlRenderTarget::GlRenderTarget(const uint32_t glFrameBufferId)
    {
        m_id = glFrameBufferId;
    }

    GlRenderTarget::GlRenderTarget()
    {
        m_id = GlState::GlGenFrameBuffer();
    }

    GlRenderTarget::~GlRenderTarget()
    {
        GlState::GlDeleteFrameBuffer(m_id);
    }

    void GlRenderTarget::StartSetting()
    {
        assert(!m_isSetting);

        UnbindFrameBufferTextures(m_colorAttachments, m_depthAttachment);

        m_colorAttachments.clear();
        m_depthAttachment.reset();

        m_isSetting = true;
        GlState::Ins()->UseGlResource(shared_from_this());
        GlState::Ins()->BindRenderTarget(shared_from_this());
    }

    void GlRenderTarget::EndSetting()
    {
        assert(m_isSetting);
        assert(!m_colorAttachments.empty() || m_depthAttachment);

        CheckAttachments(m_colorAttachments, m_depthAttachment, m_width, m_height);
        ConfigureColorTargets(m_colorAttachments);
        BindFrameBufferTextures(m_colorAttachments, m_depthAttachment);
        GlState::GlCheckFramebufferStatus();

        m_isSetting = false;
        GlState::Ins()->EndUseGlResource(shared_from_this());
        GlState::Ins()->UnBindRenderTarget(shared_from_this());
    }

    void GlRenderTarget::AddAttachment(crsp<GlTexture> attachment)
    {
        assert(m_isSetting);

        if (IsColorAttachment(attachment))
        {
            assert(!exists(m_colorAttachments, attachment));
            m_colorAttachments.push_back(attachment);
        }
        else if (IsDepthAttachment(attachment))
        {
            assert(!m_depthAttachment);
            m_depthAttachment = attachment;
        }
        else
        {
            THROW_ERROR("Invalid attachment type")
        }
    }

    void GlRenderTarget::Use()
    {
        assert(!m_isSetting && IsComplete());

        GlState::Ins()->BindRenderTarget(shared_from_this());
        GlState::GlViewport(0, 0, m_width, m_height);
    }

    void GlRenderTarget::Clear(crvec<Vec4> clearColors, const float clearDepth, const uint32_t clearType)
    {
        assert(!m_isSetting && IsComplete() && GlState::Ins()->GetGlRenderTarget() == this);
        auto needClearColor = clearType & 0b01 && !m_colorAttachments.empty() && !clearColors.empty();
        auto needClearDepth = clearType & 0b10 && m_depthAttachment;
        assert(!needClearColor || clearColors.size() == m_colorAttachments.size());

        uint32_t realClearType = 0;
        if (needClearColor)
        {
            realClearType |= 0b01;
        }
        if (needClearDepth)
        {
            realClearType |= 0b10;
        }

        ClearUnchecked(clearColors, clearDepth, realClearType);
    }
    
    void GlRenderTarget::ClearUnchecked(crvec<Vec4> clearColors, const float clearDepth, const uint32_t clearType)
    {
        assert(GlState::Ins()->GetGlRenderTarget() == this);
        
        GLuint clearBits = 0;
        
        if (clearType & 0b01)
        {
            clearBits |= GL_COLOR_BUFFER_BIT;
            for (uint32_t i = 0; i < clearColors.size(); ++i)
            {
                GlState::GlClearBufferFv(clearColors[i], i);
            }
        }

        if (clearType & 0b10)
        {
            clearBits |= GL_DEPTH_BUFFER_BIT;
            GlState::GlClearDepth(clearDepth);
        }

        GlState::GlClear(clearBits);
    }

    sp<GlRenderTarget> GlRenderTarget::GetFrameRenderTarget()
    {
        static sp<GlRenderTarget> frameRenderTarget = nullptr;
        if (!frameRenderTarget)
        {
            frameRenderTarget = msp<GlRenderTarget>(0);
            frameRenderTarget->m_width = GameFramework::Ins()->GetScreenWidth();
            frameRenderTarget->m_height = GameFramework::Ins()->GetScreenHeight();
            GetGR()->onFrameBufferResize.Add([](GLFWwindow*, const uint32_t width, const uint32_t height)
            {
                frameRenderTarget->m_width = width;
                frameRenderTarget->m_height = height;
            });
        }

        return frameRenderTarget;
    }

    bool GlRenderTarget::IsColorAttachment(crsp<GlTexture> attachment)
    {
        return attachment->GetFormat() == TextureFormat::RGBA || attachment->GetFormat() == TextureFormat::RGBA_HDR || attachment->GetFormat() == TextureFormat::DEPTH_TEX;
    }

    bool GlRenderTarget::IsDepthAttachment(crsp<GlTexture> attachment)
    {
        return attachment->GetFormat() == TextureFormat::DEPTH || attachment->GetFormat() == TextureFormat::DEPTH_STENCIL;
    }

    void GlRenderTarget::CheckAttachments(
        crvec<sp<GlTexture>> colorAttachments,
        crsp<GlTexture> depthAttachment,
        uint32_t& width,
        uint32_t& height)
    {
        width = height = 0;
        
        auto checkAttachment = [&](const GlTexture* attachment)
        {
            if (!attachment)
            {
                return;
            }
        
            if ((width != 0 && width != attachment->GetWidth()) || (height != 0 && height != attachment->GetHeight()))
            {
                THROW_ERRORF("Attachment size mismatch: %dx%d vs %dx%d", width, height, attachment->GetWidth(), attachment->GetHeight())
            }
            width = attachment->GetWidth();
            height = attachment->GetHeight();
        };
        
        for (auto& attachment : colorAttachments)
        {
            checkAttachment(attachment.get());
        }
        checkAttachment(depthAttachment.get());
    }

    void GlRenderTarget::ConfigureColorTargets(crvec<sp<GlTexture>> colorAttachments)
    {
        if (colorAttachments.empty())
        {
            GlState::GlDrawBuffer(GL_NONE);
        }
        else
        {
            auto attachmentIndices = vec<uint32_t>(colorAttachments.size());
            for (uint32_t i = 0; i < colorAttachments.size(); ++i)
            {
                attachmentIndices[i] = GL_COLOR_ATTACHMENT0 + i;
            }
            GlState::GlDrawBuffers(attachmentIndices.size(), attachmentIndices.data());
        }
    }

    void GlRenderTarget::BindFrameBufferTextures(crvec<sp<GlTexture>> colorAttachments, crsp<GlTexture> depthAttachment)
    {
        for (uint32_t i = 0; i < colorAttachments.size(); ++i)
        {
            assert(IsColorAttachment(colorAttachments[i]));
            
            GlState::GlFrameBufferTexture2D(GL_COLOR_ATTACHMENT0 + i, colorAttachments[i]->GetId());
        }
        if (depthAttachment)
        {
            assert(IsDepthAttachment(depthAttachment));
            
            auto attachmentType = GL_DEPTH_ATTACHMENT;
            if (depthAttachment->GetFormat() == TextureFormat::DEPTH_STENCIL)
            {
                attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;
            }
            
            GlState::GlFrameBufferTexture2D(attachmentType, depthAttachment->GetId());
        }
    }

    void GlRenderTarget::UnbindFrameBufferTextures(crvec<sp<GlTexture>> colorAttachments, crsp<GlTexture> depthAttachment)
    {
        for (uint32_t i = 0; i < colorAttachments.size(); ++i)
        {
            GlState::GlFrameBufferTexture2D(GL_COLOR_ATTACHMENT0 + i, GL_NONE);
        }
        if (depthAttachment)
        {
            auto attachmentType = GL_DEPTH_ATTACHMENT;
            if (depthAttachment->GetFormat() == TextureFormat::DEPTH_STENCIL)
            {
                attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;
            }
            
            GlState::GlFrameBufferTexture2D(attachmentType, GL_NONE);
        }
    }
}
