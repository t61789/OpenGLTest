#pragma once
#include "i_render_pass.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class PreparingPass final : public IRenderPass, public ControlPanelUi::UiProxy
    {
    public:
        std::string GetName() override { return "Preparing Pass";}
        
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        static void PrepareMatrices();
        static void PrepareViewport();
        static void UpdateTransforms();
        static void ClearGBuffers();
        static void PrepareLightInfos();
        static void SetAmbientColors();
        static void SetFogParams();
    };
}
