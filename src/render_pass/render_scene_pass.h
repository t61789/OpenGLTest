#pragma once
#include "RenderPass.h"

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
