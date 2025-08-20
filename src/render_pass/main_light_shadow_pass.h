#pragma once
#include "render_pass.h"

namespace op
{
    class MainLightShadowPass : public RenderPass
    {
    public:
        explicit MainLightShadowPass(RenderContext* renderContext);
        ~MainLightShadowPass() override;
        std::string GetName() override;
        void Execute() override;

    private:
        void UpdateRt();
    
        RenderTexture* m_mainLightShadowRt = nullptr;
    };
}
