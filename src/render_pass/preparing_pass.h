#pragma once
#include "i_render_pass.h"
#include "ui/control_panel_ui.h"

namespace op
{
    class PreparingPass final : public IRenderPass, public ControlPanelUi::UiProxy
    {
    public:
        PreparingPass() noexcept = default;
        ~PreparingPass() noexcept override = default;
        PreparingPass(const PreparingPass& other) = delete;
        PreparingPass(PreparingPass&& other) noexcept = delete;
        PreparingPass& operator=(const PreparingPass& other) = delete;
        PreparingPass& operator=(PreparingPass&& other) noexcept = delete;

        std::string GetName() override { return "Preparing Pass";}
        
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        void PrepareMatrices();
        void SortTransparentComps();
        static void PrepareViewport();
        static void UpdateTransforms();
        static void ClearGBuffers();
        static void PrepareLightInfos();
        static void SetAmbientColors();
        static void SetFogParams();
    };
}
