#pragma once
#include "Image.h"
#include "RenderPass.h"

class FinalBlitPass : public RenderPass
{
public:
    Material* finalBlitMat = nullptr;
    Image* lutTexture = nullptr; 
    
    FinalBlitPass();
    ~FinalBlitPass();
    
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;
};
