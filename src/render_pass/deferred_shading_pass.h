#pragma once
#include "render_pass.h"

namespace op
{
    class Mesh;

    class DeferredShadingPass : public RenderPass
    {
    public:
        DeferredShadingPass(RenderContext* renderContext);
        ~DeferredShadingPass() override;
        std::string GetName() override;
        void Execute() override;

    private:
        void UpdateRt();
    
        RenderTexture* m_shadingRt = nullptr;
        RenderTexture* m_tempPpRt0 = nullptr;
        RenderTexture* m_tempPpRt1 = nullptr;
        Mesh* m_quadMesh = nullptr;
        Material* m_deferredShadingMat = nullptr;
    };
}
