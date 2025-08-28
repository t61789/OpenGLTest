#pragma once
#include "render_pass.h"

namespace op
{
    class BatchRenderUnit;
    
    class BatchRenderPass : public RenderPass
    {
    public:
        BatchRenderPass(RenderContext* renderContext);
        ~BatchRenderPass() override;

        std::string GetName() override { return "BatchRenderPass";}
        void Execute() override;

    private:
        std::unique_ptr<BatchRenderUnit> m_batchRenderUnit;
    };
}
