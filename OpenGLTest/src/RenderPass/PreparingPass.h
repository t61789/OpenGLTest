#pragma once
#include "RenderPass.h"

class PreparingPass : public RenderPass
{
public:
    PreparingPass(RenderContext* renderContext);
    std::string GetName() override;
    void Execute() override;
};
