#pragma once
#include <string>

#include "RenderContext.h"

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual std::string GetName() = 0;
    virtual void Execute(RenderContext& renderContext) = 0;
};
