#pragma once
#include "image.h"
#include "gui.h"
#include "render_pass.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class FinalBlitPass : public RenderPass, public ControlPanelUi::UiProxy
    {
    public:
        Material* finalBlitMat = nullptr;
        Image* lutTexture = nullptr; 
    
        FinalBlitPass(RenderContext* renderContext);
        ~FinalBlitPass() override;
    
        std::string GetName() override;
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        float m_minLuminance = 0;
        float m_maxLuminance = 1;
    };
}
