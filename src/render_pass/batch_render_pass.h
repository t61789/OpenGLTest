#pragma once
#include "const.h"
#include "i_render_pass.h"
#include "render/batch_render_unit.h"

namespace op
{
    class BatchRenderPass final : public IRenderPass
    {
    public:
        BatchRenderPass();

        std::string GetName() override { return "BatchRenderPass";}
        void Execute() override;

    private:
        up<BatchRenderUnit> m_batchRenderUnit;
    };
}
