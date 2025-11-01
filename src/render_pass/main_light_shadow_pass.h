#pragma once
#include "const.h"
#include "i_render_pass.h"
#include "math/matrix4x4.h"
#include "objects/transform_comp.h"

namespace op
{
    class RenderTexture;
    class Material;

    class MainLightShadowPass final : public IRenderPass
    {
    public:
        std::string GetName() override { return "Main Light Shadow Pass"; }
        void Execute() override;
        void DrawUI();

    private:
        void UpdateRt();
        
        sp<RenderTexture> m_mainLightShadowRt = nullptr;
    };
}
