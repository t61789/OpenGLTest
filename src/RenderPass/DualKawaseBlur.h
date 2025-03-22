#pragma once

#include "RenderPass.h"
#include "UI/ControlPanelUi.h"

namespace op
{
    class DualKawaseBlur : public RenderPass, public ControlPanelUi::UiProxy
    {
    public:
        explicit DualKawaseBlur(RenderContext* renderContext);
        ~DualKawaseBlur() override;
    
        std::string GetName() override;
        void Execute() override;
        void DrawConsoleUi() override;

    private:
        int m_maxIterations = 5;
        int m_blurSize = 3;
        float m_threshold = 0.97f;
    
        Material* m_downsampleMat = nullptr;
        Material* m_upsampleMat = nullptr;
        std::vector<RenderTexture*> m_blurTextures;

        void UpdateRt(const RenderTexture* shadingRt);
        void ReleaseRt();
        int GetIteration(int originWidth, int originHeight);
    };
}
