#pragma once

#include "i_gl_resource.h"
#include "utils.h"

namespace op
{
    struct Vec4;
    class GlTexture;

    class GlRenderTarget final : public IGlResource, public std::enable_shared_from_this<GlRenderTarget>
    {
    public:
        explicit GlRenderTarget(uint32_t glFrameBufferId);
        GlRenderTarget();
        ~GlRenderTarget();
        GlRenderTarget(const GlRenderTarget& other) = delete;
        GlRenderTarget(GlRenderTarget&& other) noexcept = delete;
        GlRenderTarget& operator=(const GlRenderTarget& other) = delete;
        GlRenderTarget& operator=(GlRenderTarget&& other) noexcept = delete;

        uint32_t GetId() const { return m_id;}

        void StartSetting();
        void EndSetting();
        void AddAttachment(crsp<GlTexture> attachment);

        void Use();
        void Clear(crvec<Vec4> clearColors, float clearDepth, uint32_t clearType);
        void ClearUnchecked(crvec<Vec4> clearColors, float clearDepth, uint32_t clearType);
        bool IsComplete() const { return m_width != 0;}

        static sp<GlRenderTarget> GetFrameRenderTarget();

    private:
        uint32_t m_id;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        vec<sp<GlTexture>> m_colorAttachments;
        sp<GlTexture> m_depthAttachment = nullptr;
        bool m_isSetting = false;

        static bool IsColorAttachment(crsp<GlTexture> attachment);
        static bool IsDepthAttachment(crsp<GlTexture> attachment);
        static void CheckAttachments(crvec<sp<GlTexture>> colorAttachments, crsp<GlTexture> depthAttachment, uint32_t& width, uint32_t& height);
        static void ConfigureColorTargets(crvec<sp<GlTexture>> colorAttachments);
        static void BindFrameBufferTextures(crvec<sp<GlTexture>> colorAttachments, crsp<GlTexture> depthAttachment);
        static void UnbindFrameBufferTextures(crvec<sp<GlTexture>> colorAttachments, crsp<GlTexture> depthAttachment);
    };
}
