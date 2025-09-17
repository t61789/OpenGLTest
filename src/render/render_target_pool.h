#pragma once
#include "const.h"

namespace op
{
    class RenderTarget;
    class RenderTexture;

    struct UsingRenderTarget
    {
        UsingRenderTarget();
        explicit UsingRenderTarget(crsp<RenderTexture> rt);
        explicit UsingRenderTarget(crvecsp<RenderTexture> rts);
        ~UsingRenderTarget();
        UsingRenderTarget(const UsingRenderTarget& other) = delete;
        UsingRenderTarget(UsingRenderTarget&& other) noexcept = delete;
        UsingRenderTarget& operator=(const UsingRenderTarget& other) = delete;
        UsingRenderTarget& operator=(UsingRenderTarget&& other) noexcept = delete;
        void* operator new(size_t size) = delete;
        void* operator new[](size_t size) = delete;
        void operator delete(void* ptr) = delete;
        void operator delete[](void* ptr) = delete;

        RenderTarget* Get() const { return m_renderTarget;}

    private:
        RenderTarget* m_renderTarget = nullptr;
    };
    
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
