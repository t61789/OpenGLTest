#include "RenderTarget.h"

#include "Camera.h"
#include "Utils.h"


RenderTargetAttachment::RenderTargetAttachment(const GLuint attachmentType, const glm::vec4 clearColor, const RESOURCE_ID renderTexture):
    attachmentType(attachmentType),
    clearColor(clearColor),
    renderTexture(renderTexture)
{
}

RenderTarget::RenderTarget(const std::vector<RenderTargetAttachment>& attachments, const int colorAttachmentsNum, const std::string& name)
{
    if(attachments.empty())
    {
        throw std::runtime_error("创建了一个空的RenderTarget");
    }

    this->name = name;
    width = height = 0;
    glGenFramebuffers(1, &frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    
    this->renderTargetAttachments = attachments;
    this->colorAttachmentsNum = colorAttachmentsNum;

    if(colorAttachmentsNum == 0)
    {
        glDrawBuffer(GL_NONE);
    }
    else
    {
        auto buffers = new GLuint[colorAttachmentsNum];
        for (int i = 0; i < colorAttachmentsNum; ++i)
        {
            buffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(colorAttachmentsNum, buffers);
        delete[] buffers;
    }

    rebindAttachments();
}

RenderTarget::~RenderTarget()
{
    glDeleteFramebuffers(1, &frameBufferId);
}

void RenderTarget::use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    glViewport(0, 0, width, height);
}

void RenderTarget::clear(const GLuint clearBits)
{
    use();

    if((clearBits & GL_COLOR_BUFFER_BIT) != 0)
    {
        for (int i = 0; i < colorAttachmentsNum; ++i)
        {
            glm::vec4 clearColor = renderTargetAttachments[i].clearColor;
            GLfloat glClearColor[] = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
            glClearBufferfv(GL_COLOR, i, glClearColor);
        }
    }

    if((clearBits & GL_DEPTH_BUFFER_BIT) != 0)
    {
        glClearDepth(1.0f);
    }
    
    glClear(clearBits);
}

void RenderTarget::rebindAttachments()
{
    if(renderTargetAttachments.empty())
    {
        return;
    }

    width = height = 0;
    for (int i = 0; i < renderTargetAttachments.size(); ++i)
    {
        auto renderTexture = ResourceMgr::GetPtr<RenderTexture>(renderTargetAttachments[i].renderTexture);
        if(renderTexture == nullptr)
        {
            throw std::runtime_error("绑定Attachments时出错，RenderTexture[" + std::to_string(renderTargetAttachments[i].renderTexture) + "]不存在");
        }

        if(i == 0)
        {
            width = renderTexture->width;
            height = renderTexture->height;
        }
        else if(width != renderTexture->width || height != renderTexture->height)
        {
            throw std::runtime_error("绑定Attachments时出错，RenderTexture[" + std::to_string(renderTargetAttachments[i].renderTexture) + "]的尺寸不一致");
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    Utils::CheckGlError("绑定FrameBuffer");
    
    for (auto& renderTargetAttachment : renderTargetAttachments)
    {
        auto renderTexture = ResourceMgr::GetPtr<RenderTexture>(renderTargetAttachment.renderTexture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, renderTargetAttachment.attachmentType, GL_TEXTURE_2D, renderTexture->glTextureId, 0);
        Utils::CheckGlError("指定FrameBuffer附件");
    }

    _checkRenderTargetComplete();
}

void RenderTarget::_checkRenderTargetComplete()
{
    auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("FrameBufferAttachment绑定失败：" + std::to_string(frameBufferStatus));
    }
}

void RenderTarget::UseScreenTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::ClearFrameBuffer(const GLuint frameBuffer, const glm::vec4 clearColor, const GLuint clearBits)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(clearBits);
}
