#pragma once
#include "i_render_pass.h"
#include "glad/glad.h"

namespace op
{
    class RenderScenePass : public IRenderPass
    {
    public:
        std::string GetName() override { return "Render Scene Pass";}
        void Execute() override;
    };
}
