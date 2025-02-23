#pragma once
#include "RenderPass.h"

class KawaseBlur : public RenderPass
{
public:
    KawaseBlur();
    ~KawaseBlur() override;
    
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;

    RenderTexture* rt = nullptr;

private:
    Material* kawaseBlitMat = nullptr;
};
