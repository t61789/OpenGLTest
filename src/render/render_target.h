#pragma once

#include "utils.h"

namespace op
{
    struct Vec4;
    class GlRenderTarget;
    class RenderTexture;
    class RenderTarget;

    using UsingRenderTargetObj = UsingObjectT<RenderTarget*>;

    class RenderTarget final
    {
    public:
        explicit RenderTarget(crvecsp<RenderTexture> renderTextures);
        ~RenderTarget();
        RenderTarget(const RenderTarget& other) = delete;
        RenderTarget(RenderTarget&& other) noexcept = delete;
        RenderTarget& operator=(const RenderTarget& other) = delete;
        RenderTarget& operator=(RenderTarget&& other) noexcept = delete;

        void Use();
        void Clear(crvec<Vec4> colors);
        void Clear(float depth);
        void Clear(crvec<Vec4> colors, float depth);
        void ClearUnchecked(crvec<Vec4> colors, float depth);

        static UsingRenderTargetObj Using();
        static UsingRenderTargetObj Using(crvecsp<RenderTexture> rts);
        static UsingRenderTargetObj Using(crsp<RenderTexture> rt);

    private:
        bool m_dirty = true;
        vecsp<RenderTexture> m_renderTextures;
        EventHandler m_onAnyRtResizedHandler = 0;
        
        sp<GlRenderTarget> m_glRenderTarget;

        void OnAnyRtResized();
        void Recreate();
    };
}
