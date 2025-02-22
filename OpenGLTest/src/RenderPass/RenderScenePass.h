#pragma once
#include "RenderPass.h"

class RenderScenePass : public RenderPass
{
public:
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;
};
