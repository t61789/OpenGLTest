#include "kawase_blur.h"

#include <tracy/Tracy.hpp>

#include "imgui.h"

#include "gui.h"
#include "render_texture.h"
#include "material.h"
#include "rendering_utils.h"

namespace op
{
    KawaseBlur::KawaseBlur(RenderContext* renderContext) : RenderPass(renderContext)
    {
        rt = new RenderTexture(RenderTextureDescriptor(
            2,
            2,
            RenderTextureFormat::RGBA,
            Bilinear,
            Clamp));
        INCREF(rt);

        m_kawaseBlitMat = Material::CreateEmptyMaterial("shaders/kawase_blit.glsl");
        INCREF(m_kawaseBlitMat);
    }

    KawaseBlur::~KawaseBlur()
    {
        DECREF(rt);
        DECREF(m_kawaseBlitMat);
    }

    std::string KawaseBlur::GetName()
    {
        return "Kawase Blur";
    }

    void KawaseBlur::Execute()
    {
        ZoneScoped;
        
        auto shadingRt = m_renderContext->GetRt("_ShadingBufferTex");
        auto rt0 = shadingRt;
        auto rt1 = m_renderContext->GetRt("_TempPpRt0");

        if (!rt0 || !rt1)
        {
            return;
        }

        for (int i = 0; i < m_iteration; ++i)
        {
            m_kawaseBlitMat->SetTextureValue("_MainTex", rt0);
            m_kawaseBlitMat->SetFloatValue("_Iterations", static_cast<float>(i));

            RenderingUtils::Blit(rt0, rt1, m_kawaseBlitMat);

            auto temp = rt0;
            rt0 = rt1;
            rt1 = temp;
        }

        if (m_iteration % 2)
        {
            RenderingUtils::Blit(rt0, shadingRt, nullptr);
        }
    }

    void KawaseBlur::DrawConsoleUi()
    {
        ImGui::SliderInt("Kawase Blur Iterations", &m_iteration, 0, 16);
    }
}
