#pragma once

#include <vec4.hpp>

#include "RenderTexture.h"
#include "ResourceMgr.h"

class RenderTargetAttachment
{
public:
    GLuint attachmentType;
    glm::vec4 clearColor;
    RESOURCE_ID renderTexture;

    RenderTargetAttachment(GLuint attachmentType, glm::vec4 clearColor, RESOURCE_ID renderTexture);
};

class RenderTarget : public ResourceBase
{
public:
    GLuint frameBufferId;

    std::string name = "Unnamed RenderTarget";

    size_t width;
    size_t height;

    int colorAttachmentsNum;
    
    RenderTarget(const std::vector<RenderTargetAttachment>& attachments, int colorAttachmentsNum, const std::string& name = "Unnamed RenderTarget");
    ~RenderTarget();

    void use();
    void clear(GLuint clearBits);
    void rebindAttachments();

    static void UseScreenTarget();
    static void ClearFrameBuffer(GLuint frameBuffer, glm::vec4 clearColor, GLuint clearBits);

private:
    std::vector<RenderTargetAttachment> renderTargetAttachments;

    void _checkRenderTargetComplete();
};
