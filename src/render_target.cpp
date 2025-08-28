#include "render_target.h"

#include "render_texture.h"
#include "shared_object.h"
#include "game_framework.h"
#include "game_resource.h"
#include "utils.h"

namespace op
{
    bool RenderTargetAttachment::Equals(const RenderTargetAttachment* other)
    {
        return attachmentType == other->attachmentType &&
            renderTexture == other->renderTexture;
    }

    RenderTargetAttachment::RenderTargetAttachment(GLuint attachmentType, RenderTexture* renderTexture):
        attachmentType(attachmentType),
        renderTexture(renderTexture)
    {
    }

    RenderTargetDesc::~RenderTargetDesc()
    {
        for (auto renderTargetAttachment : colorAttachments)
        {
            delete renderTargetAttachment;
        }

        delete depthAttachment;
    }

    void RenderTargetDesc::SetColorAttachment(RenderTexture* rt)
    {
        SetColorAttachment(0, rt);
    }

    void RenderTargetDesc::SetColorAttachment(const int index, RenderTexture* rt)
    {
        if (colorAttachments.size() <= index)
        {
            colorAttachments.resize(index + 1, nullptr);
        }

        delete colorAttachments[index];

        colorAttachments[index] = new RenderTargetAttachment(GL_COLOR_ATTACHMENT0 + index, rt);
    }

    void RenderTargetDesc::SetDepthAttachment(RenderTexture* rt, const bool hasStencil)
    {
        delete depthAttachment;

        GLuint attachmentType = GL_DEPTH_ATTACHMENT;
        if (hasStencil)
        {
            attachmentType = GL_DEPTH_STENCIL_ATTACHMENT;
        }
        
        depthAttachment = new RenderTargetAttachment(attachmentType, rt);
    }

    std::vector<RenderTarget*> RenderTarget::m_renderTargetsPool;
    int RenderTarget::m_lastClearFrame = 0;
    int RenderTarget::m_renderTargetTimeout = 10;

    RenderTarget::RenderTarget(const RenderTargetDesc& desc)
    {
        name = desc.name;
        width = height = 0;
        m_resizeHandler = GenEventHandler();
        
        LoadAttachments(desc);
        
        // 创建framebuffer
        glGenFramebuffers(1, &frameBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

        // 配置glDrawBuffer
        if(m_colorAttachments.empty())
        {
            glDrawBuffer(GL_NONE);
        }
        else
        {
            auto colorAttachmentsNum = static_cast<int>(m_colorAttachments.size());
            auto buffers = new GLuint[colorAttachmentsNum];
            
            for (int i = 0; i < colorAttachmentsNum; ++i)
            {
                buffers[i] = GL_COLOR_ATTACHMENT0 + i;
            }
            glDrawBuffers(colorAttachmentsNum, buffers);

            delete[] buffers;
        }
    }

    RenderTarget::~RenderTarget()
    {
        ReleaseAttachments();
        
        glDeleteFramebuffers(1, &frameBufferId);
    }

    void RenderTarget::Use()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
        glViewport(0, 0, width, height);
    }

    void RenderTarget::Clear(const float depth)
    {
        Clear(std::vector<Vec4>(), depth, 0b10);
    }

    void RenderTarget::Clear(const Vec4 color)
    {
        Clear(std::vector{color}, 1.0f, 0b01);
    }

    void RenderTarget::Clear(const Vec4 color, const float depth)
    {
        Clear(std::vector{color}, depth, 0b11);
    }

    void RenderTarget::Clear(const std::vector<Vec4>& colors)
    {
        Clear(colors, 1.0f, 0b01);
    }

    void RenderTarget::Clear(const std::vector<Vec4>& colors, const float depth)
    {
        Clear(colors, depth, 0b11);
    }

    void RenderTarget::Clear(const std::vector<Vec4>& colors, const float depth, const int clearType)
    {
        Use();
        
        GLuint clearBits = 0;
        
        if (clearType & 0b01 && !colors.empty())
        {
            clearBits |= GL_COLOR_BUFFER_BIT;
            for (int i = 0; i < colors.size(); ++i)
            {
                Vec4 clearColor = colors[i];
                GLfloat glClearColor[] = {clearColor.x, clearColor.y, clearColor.z, clearColor.w};
                glClearBufferfv(GL_COLOR, i, glClearColor);
            }
        }

        if (clearType & 0b10)
        {
            clearBits |= GL_DEPTH_BUFFER_BIT;
            glClearDepth(depth);
        }
        
        glClear(clearBits);
    }

    void RenderTarget::LoadAttachments(const RenderTargetDesc& desc)
    {
        RenderTargetAttachment* rta = nullptr;
        
        for (auto colorAttachment : desc.colorAttachments)
        {
            rta = new RenderTargetAttachment(colorAttachment->attachmentType, colorAttachment->renderTexture);
            INCREF(rta->renderTexture)
            rta->renderTexture->onResize->Add(this, &RenderTarget::SetDirty, m_resizeHandler);
            m_colorAttachments.push_back(rta);
        }

        rta = nullptr;
        if (desc.depthAttachment)
        {
            rta = new RenderTargetAttachment(desc.depthAttachment->attachmentType, desc.depthAttachment->renderTexture);
            rta->renderTexture->onResize->Add(this, &RenderTarget::SetDirty, m_resizeHandler);
            INCREF(rta->renderTexture)
        }
        m_depthAttachment = rta;
    }

    void RenderTarget::ReleaseAttachments()
    {
        for (auto colorAttachment : m_colorAttachments)
        {
            colorAttachment->renderTexture->onResize->Remove(m_resizeHandler);
            DECREF(colorAttachment->renderTexture)
            delete colorAttachment;
        }

        if (m_depthAttachment)
        {
            m_depthAttachment->renderTexture->onResize->Remove(m_resizeHandler);
            DECREF(m_depthAttachment->renderTexture)
            delete m_depthAttachment;
        }
    }

    bool RenderTarget::AttachmentsMatched(const RenderTargetDesc& desc)
    {
        if (m_colorAttachments.size() != desc.colorAttachments.size())
        {
            return false;
        }

        for (int i = 0; i < m_colorAttachments.size(); ++i)
        {
            if (!m_colorAttachments[i]->Equals(desc.colorAttachments[i]))
            {
                return false;
            }
        }
        
        if (m_depthAttachment == desc.depthAttachment)
        {
            return true;
        }

        if (m_depthAttachment == nullptr || desc.depthAttachment == nullptr)
        {
            return false;
        }

        return m_depthAttachment->Equals(desc.depthAttachment); 
    }

    void RenderTarget::SetDirty()
    {
        m_dirty = true;
    }

    void RenderTarget::RebindAttachments()
    {
        // 检查color attachment的尺寸是否一致
        width = height = 0;
        for (int i = 0; i <= m_colorAttachments.size(); ++i)
        {
            RenderTargetAttachment* attachment;
            if (i == m_colorAttachments.size())
            {
                attachment = m_depthAttachment;
                if (!attachment)
                {
                    break;
                }
            }
            else
            {
                attachment = m_colorAttachments[i];
            }
            
            auto rt = attachment->renderTexture;
            if(i == 0)
            {
                width = rt->width;
                height = rt->height;
            }
            else if(width != rt->width || height != rt->height)
            {
                THROW_ERRORF("绑定Attachments时出错，RenderTexture[%s]的尺寸不一致", rt->desc.name.c_str())
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
        GL_CHECK_ERROR(绑定FrameBuffer)

        // 绑定color attachment
        for (auto colorAttachment : m_colorAttachments)
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                colorAttachment->attachmentType,
                GL_TEXTURE_2D,
                colorAttachment->renderTexture->glTextureId,
                0);
            GL_CHECK_ERROR(指定FrameBuffer附件)
        }

        // 绑定depth attachment
        if (m_depthAttachment)
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                m_depthAttachment->attachmentType,
                GL_TEXTURE_2D,
                m_depthAttachment->renderTexture->glTextureId,
                0);
        }

        CheckRenderTargetComplete();
    }

    RenderTarget* RenderTarget::Get(RenderTexture* colorAttachment, RenderTexture* depthAttachment, const bool hasStencil)
    {
        RenderTargetDesc desc;
        if (colorAttachment != nullptr)
        {
            desc.SetColorAttachment(colorAttachment);
        }
        if (depthAttachment != nullptr)
        {
            desc.SetDepthAttachment(depthAttachment, hasStencil);
        }
        return Get(desc);
    }

    RenderTarget* RenderTarget::Get(const RenderTargetDesc& desc)
    {
        RenderTarget* result = nullptr;
        for (auto renderTarget : m_renderTargetsPool)
        {
            if (!renderTarget->AttachmentsMatched(desc))
            {
                continue;
            }

            result = renderTarget;
            break;
        }

        if (!result)
        {
            result = new RenderTarget(desc);
            m_renderTargetsPool.push_back(result);
        }

        result->name = desc.name;
        result->m_lastUseFrame = GetGR()->time.frame;
        if (result->m_dirty)
        {
            result->RebindAttachments();
            result->m_dirty = false;
        }
        ClearUnusedRenderTargets();
        return result;
    }

    void RenderTarget::UseScreenTarget()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderTarget::CheckRenderTargetComplete()
    {
        auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            THROW_ERRORF("FrameBufferAttachment绑定失败：%s", std::to_string(frameBufferStatus).c_str())
        }
    }

    void RenderTarget::ClearFrameBuffer(const GLuint frameBuffer, const Vec4& clearColor, const GLuint clearBits)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(clearBits);
    }

    void RenderTarget::ClearAllCache()
    {
        for (auto& renderTarget : m_renderTargetsPool)
        {
            delete renderTarget;
        }
        m_renderTargetsPool.clear();
    }

    void RenderTarget::ClearUnusedRenderTargets()
    {
        int curFrame = GetGR()->time.frame;
        if (curFrame - m_lastClearFrame < m_renderTargetTimeout)
        {
            return;
        }
        m_lastClearFrame = curFrame;
        
        for (int i = 0; i < m_renderTargetsPool.size(); ++i)
        {
            auto renderTarget = m_renderTargetsPool[i];
            if (curFrame - renderTarget->m_lastUseFrame >= m_renderTargetTimeout)
            {
                delete renderTarget;
                m_renderTargetsPool.erase(m_renderTargetsPool.begin() + i);
                i--;
            }
        }
    }
}
