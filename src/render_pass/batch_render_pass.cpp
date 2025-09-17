#include "batch_render_pass.h"

#include "render_context.h"
#include "render/batch_render_unit.h"
#include "render/render_target_pool.h"

namespace op
{
    BatchRenderPass::BatchRenderPass()
    {
        m_batchRenderUnit = mup<BatchRenderUnit>();
    }

    void BatchRenderPass::Execute()
    {
        auto usingGBufferRenderTarget = GetRC()->UsingGBufferRenderTarget();
        
        m_batchRenderUnit->Execute();
    }
}
