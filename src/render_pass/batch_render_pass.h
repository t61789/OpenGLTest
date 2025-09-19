#pragma once
#include "i_render_pass.h"

namespace op
{
    class BatchRenderPass final : public IRenderPass
    {
    public:
        std::string GetName() override { return "Batch Render Pass";}
        void Execute() override;
    };
}
