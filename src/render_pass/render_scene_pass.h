#pragma once
#include "render_pass.h"

namespace op
{
    class RenderScenePass : public RenderPass
    {
    public:
        explicit RenderScenePass(RenderContext* renderContext);
        std::string GetName() override;
        void Execute() override;
    };
}
