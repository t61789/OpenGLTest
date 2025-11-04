#pragma once
#include "i_render_pass.h"

namespace op
{
    class TransparentPass final : public IRenderPass
    {
    public:
        std::string GetName() override { return "Transparent Pass"; }
        
        void Execute() override;
    };
}
