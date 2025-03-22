#pragma once
#include "RenderPass.h"
#include "UI/ControlPanelUi.h"

namespace op
{
    class PreparingPass : public RenderPass, public ControlPanelUi::UiProxy
    {
    public:
        PreparingPass(RenderContext* renderContext);
        std::string GetName() override;
        void Execute() override;

        void DrawConsoleUi() override;

    private:
        void PrepareLightInfos();
    };
}
