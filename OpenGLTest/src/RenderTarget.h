#pragma once

#include <vec4.hpp>

#include "RenderTexture.h"
#include "SharedObject.h"

class RenderTargetAttachment
{
public:
    GLuint attachmentType;
    glm::vec4 clearColor;
    RenderTexture* renderTexture;

    RenderTargetAttachment(GLuint attachmentType, glm::vec4 clearColor, RenderTexture* renderTexture);
    ~RenderTargetAttachment();
};

class RenderTarget : public SharedObject
{
public:
    GLuint frameBufferId;

    std::string name = "Unnamed RenderTarget";

    size_t width;
    size_t height;

    int colorAttachmentsNum;
    
    RenderTarget(const std::vector<RenderTargetAttachment*>& attachments, int colorAttachmentsNum, const std::string& name = "Unnamed RenderTarget");
    ~RenderTarget();

    void Use();
    void Clear(GLuint clearBits);
    void RebindAttachments();

    static void UseScreenTarget();
    static void ClearFrameBuffer(GLuint frameBuffer, glm::vec4 clearColor, GLuint clearBits);

private:
    std::vector<RenderTargetAttachment*> renderTargetAttachments;

    void CheckRenderTargetComplete();
};
