#pragma once
#include "RenderPass/RenderPass.h"

class DeferredShadingPass : public RenderPass
{
public:
    DeferredShadingPass();
    ~DeferredShadingPass() override;
    std::string GetName() override;
    void Execute(RenderContext& renderContext) override;

private:
    void UpdateRt(const RenderContext& renderContext);
    
    RenderTexture* m_shadingRt = nullptr;
    Mesh* m_quadMesh = nullptr;
    Material* m_deferredShadingMat = nullptr;
};
