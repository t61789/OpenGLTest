#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "event.h"
#include "glad/glad.h"
#include "math/math.h"

namespace op
{
    class RenderTexture;

    class RenderTargetAttachment
    {
    public:
        GLuint attachmentType;
        RenderTexture* renderTexture;

        bool Equals(const RenderTargetAttachment* other);

        RenderTargetAttachment(GLuint attachmentType, RenderTexture* renderTexture);
    };

    class RenderTargetDesc
    {
    public:
        std::string name;
        
        RenderTargetDesc() = default;
        RenderTargetDesc(const RenderTargetDesc&) = default;
        RenderTargetDesc& operator=(const RenderTargetDesc&) = default;
        ~RenderTargetDesc();
        
        void SetColorAttachment(RenderTexture* rt);
        void SetColorAttachment(int index, RenderTexture* rt);
        void SetDepthAttachment(RenderTexture* rt, bool hasStencil = false);

        std::vector<RenderTargetAttachment*> colorAttachments;
        RenderTargetAttachment* depthAttachment = nullptr;
    };

    class RenderTarget
    {
    public:
        std::string name;
        
        GLuint frameBufferId;

        int width;
        int height;

        void Use();
        void Clear(float depth);
        void Clear(Vec4 color);
        void Clear(Vec4 color, float depth);
        void Clear(const std::vector<Vec4>& colors);
        void Clear(const std::vector<Vec4>& colors, float depth);
        void RebindAttachments();

        static RenderTarget* Get(RenderTexture* colorAttachment, RenderTexture* depthAttachment, bool hasStencil = false);
        static RenderTarget* Get(const RenderTargetDesc& desc);
        static void UseScreenTarget();
        static void ClearFrameBuffer(GLuint frameBuffer, const Vec4& clearColor, GLuint clearBits);
        static void ClearAllCache();

    private:
        int m_lastUseFrame = 0;
        bool m_dirty = true;

        EventHandler m_resizeHandler = 0;

        std::vector<RenderTargetAttachment*> m_colorAttachments;
        RenderTargetAttachment* m_depthAttachment = nullptr;

        RenderTarget(const RenderTargetDesc& desc);
        ~RenderTarget();
        
        void Clear(const std::vector<Vec4>& colors, float depth, int clearType);
        void LoadAttachments(const RenderTargetDesc& desc);
        void ReleaseAttachments();
        bool AttachmentsMatched(const RenderTargetDesc& desc);
        void SetDirty();

        static void CheckRenderTargetComplete();

        static std::vector<RenderTarget*> m_renderTargetsPool;
        static int m_lastClearFrame;
        static int m_renderTargetTimeout;
        static void ClearUnusedRenderTargets();
    };
}
