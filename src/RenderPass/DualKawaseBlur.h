#pragma once

#include "Gui.h"
#include "RenderPass.h"

class DualKawaseBlur : public RenderPass, public IGui
{
public:
    explicit DualKawaseBlur(RenderContext* renderContext);
    ~DualKawaseBlur() override;
    
    std::string GetName() override;
    void Execute() override;
    void OnDrawConsoleGui() override;

private:
    int m_maxIterations = 4;
    int m_blurSize = 1;
    
    Material* m_downsampleMat = nullptr;
    Material* m_upsampleMat = nullptr;
    std::vector<RenderTexture*> m_blurTextures;

    void UpdateRt(const RenderTexture* shadingRt);
    void ReleaseRt();
    int GetIteration(int originWidth, int originHeight);
};
