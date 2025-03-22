#pragma once

#include <functional>

#include "RenderPass.h"
#include "UI/ControlPanelUi.h"

class KawaseBlur : public RenderPass, public ControlPanelUi::UiProxy
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
    
    Material* m_kawaseBlitMat = nullptr;
};
