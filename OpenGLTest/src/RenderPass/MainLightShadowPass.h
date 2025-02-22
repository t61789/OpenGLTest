#pragma once
#include "RenderPass.h"

class MainLightShadowPass : public RenderPass
{
public:
    Material* m_drawShadowMat = nullptr;
    
    MainLightShadowPass();
    ~MainLightShadowPass() override;
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;

private:
    void UpdateRt(const RenderContext& renderContext);
    
    RenderTexture* m_mainLightShadowRt = nullptr;
};
