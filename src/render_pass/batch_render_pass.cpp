#include "batch_render_pass.h"

#include "render/batch_render_unit.h"

namespace op
{
    BatchRenderPass::BatchRenderPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        m_batchRenderUnit = std::make_unique<BatchRenderUnit>();
    }

    BatchRenderPass::~BatchRenderPass() = default;

    void BatchRenderPass::Execute()
    {
        m_batchRenderUnit->Execute();
    }
}
