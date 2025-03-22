#pragma once
#include "Gui.h"
#include "RenderPass.h"

class PreparingPass : public RenderPass, public IGui
{
public:
    PreparingPass(RenderContext* renderContext);
    std::string GetName() override;
    void Execute() override;

    void OnDrawConsoleGui() override;

private:
    void PrepareLightInfos();
};
