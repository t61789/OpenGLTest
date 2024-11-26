#pragma once

#include <vec4.hpp>

#include "RenderTexture.h"
#include "ResourceMgr.h"

class RenderTargetAttachment
{
public:
    GLuint attachmentType;
    glm::vec4 clearColor;
    RenderTextureDescriptor desc;

    RenderTargetAttachment(GLuint attachmentType, glm::vec4 clearColor, RenderTextureDescriptor desc);
};

class RenderTarget : public ResourceBase
{
public:
    GLuint frameBufferId;

    size_t width;
    size_t height;

    int colorAttachmentsNum;
    
    RenderTarget(const std::vector<RenderTargetAttachment>& colorAttachments, int colorAttachmentsNum);
    ~RenderTarget();

    void use();
    void clear(GLuint clearBits);
    bool createAttachmentsRt(size_t width, size_t height);
    bool checkRenderTargetComplete();
    RESOURCE_ID getRenderTexture(int index);

    static void UseScreenTarget();
    static void ClearFrameBuffer(GLuint frameBuffer, glm::vec4 clearColor, GLuint clearBits);

private:
    std::vector<RenderTargetAttachment> renderTargetAttachments;
    std::vector<RESOURCE_ID> attachments;

    void _destroyAttachmentsRt();
};
