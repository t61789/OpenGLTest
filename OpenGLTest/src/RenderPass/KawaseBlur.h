#pragma once
#include "RenderPass.h"

class KawaseBlur : RenderPass
{
public:
    void Execute(RenderContext& renderContext) override;   
};
