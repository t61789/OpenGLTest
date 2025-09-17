#pragma once

#include "i_render_pass.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class DualKawaseBlur : public IRenderPass, public ControlPanelUi::UiProxy
    {
    public:
        explicit DualKawaseBlur();
        ~DualKawaseBlur() override;
    
        std::string GetName() override;
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        int m_maxIterations = 5;
        int m_blurSize = 3;
        float m_threshold = 0.97f;
    
        // Material* m_downsampleMat = nullptr;
        // Material* m_upsampleMat = nullptr;
        std::vector<RenderTexture*> m_blurTextures;

        void UpdateRt(const RenderTexture* shadingRt);
        void ReleaseRt();
        uint32_t GetIteration(uint32_t originWidth, uint32_t originHeight);
    };
}
