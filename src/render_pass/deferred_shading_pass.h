#pragma once
#include "const.h"
#include "i_render_pass.h"

namespace op
{
    class RenderTexture;
    class Material;
    class Mesh;

    class DeferredShadingPass final : public IRenderPass
    {
    public:
        DeferredShadingPass();
        std::string GetName() override;
        void Execute() override;

    private:
        void UpdateRt();
    
        sp<RenderTexture> m_shadingRt = nullptr;
        sp<Material> m_deferredShadingMat = nullptr;
    };
}
