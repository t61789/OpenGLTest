#pragma once
#include "const.h"

namespace op
{
    class RenderTarget;
    class RenderTexture;

    class RenderTargetPool
    {
        friend struct UsingRenderTarget;
        
    public:
        RenderTarget* Push(crvecsp<RenderTexture> rts);
        void Pop();
        
        void TryRecycle();

    private:
        struct RenderTargetInfo
        {
            uint32_t usingFrame;
            sp<RenderTarget> renderTarget;
        };

        umap<size_t, RenderTargetInfo> m_renderTargetPool;
        vecsp<RenderTarget> m_renderTargetStack;

        sp<RenderTarget> Get(crvecsp<RenderTexture> rts);
        void AddRenderTarget(crvecsp<RenderTexture> rts);
        
        static size_t GetHash(crvecsp<RenderTexture> rts);
    };
}
