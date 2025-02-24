#pragma once
#include "RenderPass.h"

class RenderScenePass : public RenderPass
{
public:
    explicit RenderScenePass(RenderContext* renderContext);
    std::string GetName() override;
    void Execute() override;
};
