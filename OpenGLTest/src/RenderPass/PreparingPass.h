#pragma once
#include "RenderPass.h"

class PreparingPass : public RenderPass
{
public:
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;
};
