#pragma once
#include "Image.h"
#include "Gui.h"
#include "RenderPass.h"

class FinalBlitPass : public RenderPass, public IGui
{
public:
    Material* finalBlitMat = nullptr;
    Image* lutTexture = nullptr; 
    
    FinalBlitPass(RenderContext* renderContext);
    ~FinalBlitPass() override;
    
    std::string GetName() override;
    void Execute() override;
    void OnDrawConsoleGui() override;

private:
    float m_minLuminance = 0;
    float m_maxLuminance = 1;
};
