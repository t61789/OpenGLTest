#pragma once

#include <functional>

#include "i_render_pass.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class KawaseBlur : public IRenderPass, public ControlPanelUi::UiProxy
    {
    public:
        RenderTexture* rt = nullptr;
    
        KawaseBlur(RenderContext* renderContext);
        ~KawaseBlur() override;
    
        std::string GetName() override;
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        int m_iteration = 2;
    
        // Material* m_kawaseBlitMat = nullptr;
    };
}
