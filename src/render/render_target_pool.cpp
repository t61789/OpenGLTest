#include "render_target_pool.h"

#include "game_resource.h"
#include "render_context.h"
#include "render_target.h"

namespace op
{
    RenderTarget* RenderTargetPool::Push(crvecsp<RenderTexture> rts)
    {
        assert(m_renderTargetStack.size() < 10);

        if (LOG_RENDER_TARGET_STACK_NOT_EMPTY && !m_renderTargetStack.empty())
        {
            log_warning("Render target stack is not empty");
        }
        
        const auto& renderTarget = Get(rts);
        m_renderTargetStack.push_back(renderTarget);
        renderTarget->Use();
        return renderTarget.get();
    }

    void RenderTargetPool::Pop()
    {
        m_renderTargetStack.pop_back();
        if (!m_renderTargetStack.empty())
        {
            m_renderTargetStack.back()->Use();
        }
    }

    sp<RenderTarget> RenderTargetPool::Get(crvecsp<RenderTexture> rts)
    {
        auto hash = GetHash(rts);

        auto it = m_renderTargetPool.find(hash);
        if (it != m_renderTargetPool.end())
        {
            it->second.usingFrame = GetGR()->time.frame;
            return it->second.renderTarget;
        }
        
        AddRenderTarget(rts);
        return m_renderTargetPool.at(hash).renderTarget;
    }

    void RenderTargetPool::TryRecycle()
    {
        static vec<size_t> toRemove;
        toRemove.clear();
        
        auto timeOutFrame = 5u;

        for (auto& [hash, info] : m_renderTargetPool)
        {
            if (info.usingFrame + timeOutFrame < static_cast<uint32_t>(GetGR()->time.frame))
            {
                toRemove.push_back(hash);
            }
        }

        for (auto hash : toRemove)
        {
            m_renderTargetPool.erase(hash);
        }
    }

    void RenderTargetPool::AddRenderTarget(crvecsp<RenderTexture> rts)
    {
        RenderTargetInfo result;

        result.usingFrame = GetGR()->time.frame;
        result.renderTarget = msp<RenderTarget>(rts);

        m_renderTargetPool[GetHash(rts)] = std::move(result);
    }

    size_t RenderTargetPool::GetHash(crvecsp<RenderTexture> rts)
    {
        size_t seed = rts.size();
        for (const auto& value : rts)
        {
            seed ^= std::hash<uint64_t>{}(reinterpret_cast<size_t>(value.get())) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
}
