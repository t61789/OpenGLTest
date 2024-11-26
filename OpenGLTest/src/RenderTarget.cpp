#include "RenderTarget.h"

#include "Camera.h"
#include "Utils.h"


RenderTargetAttachment::RenderTargetAttachment(const GLuint attachmentType, const glm::vec4 clearColor,RenderTextureDescriptor desc):
    attachmentType(attachmentType),
    clearColor(clearColor),
    desc(std::move(desc))
{
}

RenderTarget::RenderTarget(const std::vector<RenderTargetAttachment>& colorAttachments, const int colorAttachmentsNum)
{
    width = 0;
    height = 0;
    glGenFramebuffers(1, &frameBufferId);
    this->renderTargetAttachments = colorAttachments;

    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    auto buffers = new GLuint[colorAttachmentsNum];
    for (int i = 0; i < colorAttachmentsNum; ++i)
    {
        buffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(colorAttachmentsNum, buffers);
    delete[] buffers;
}

RenderTarget::~RenderTarget()
{
    _destroyAttachmentsRt();
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

    glClear(clearBits);
}

bool RenderTarget::createAttachmentsRt(const size_t width, const size_t height)
{
    if(renderTargetAttachments.empty())
    {
        return true;
    }

    if(width == this->width && height == this->height)
    {
        return true;
    }

    this->width = width;
    this->height = height;

    _destroyAttachmentsRt();

    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

    for (auto& renderTargetAttachment : renderTargetAttachments)
    {
        auto desc = renderTargetAttachment.desc;
        desc.width = width;
        desc.height = height;
        auto rt = new RenderTexture(desc);
        attachments.push_back(rt->id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, renderTargetAttachment.attachmentType, GL_TEXTURE_2D, rt->glTextureId, 0);
    }

    return checkRenderTargetComplete();
}

bool RenderTarget::checkRenderTargetComplete()
{
    auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        Utils::Log("FrameBufferAttachment绑定失败：" + std::to_string(frameBufferStatus), Error);
        return false;
    }
    return true;
}

RESOURCE_ID RenderTarget::getRenderTexture(const int index)
{
    if(index < 0 || index >= attachments.size())
    {
        return UNDEFINED_RESOURCE;
    }

    return attachments[index];
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

void RenderTarget::_destroyAttachmentsRt()
{
    for (auto& attachment : attachments)
    {
        ResourceMgr::DeleteResource(attachment);
    }
    attachments.clear();
}

