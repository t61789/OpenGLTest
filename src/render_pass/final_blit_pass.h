#pragma once
#include "i_render_pass.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class Image;

    class FinalBlitPass : public IRenderPass, public ControlPanelUi::UiProxy
    {
    public:
        FinalBlitPass();
    
        std::string GetName() override { return "Final Blit Pass";}
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        float m_minLuminance = 0;
        float m_maxLuminance = 1;

        sp<Material> m_finalBlitMat = nullptr;
        sp<Image> m_lutTexture = nullptr;
    };
}
