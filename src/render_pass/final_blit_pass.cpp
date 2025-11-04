#include "final_blit_pass.h"

#include <tracy/Tracy.hpp>

#include "built_in_res.h"
#include "image.h"
#include "material.h"
#include "rendering_utils.h"
#include "render_context.h"
#include "render/gl/gl_state.h"

namespace op
{
    FinalBlitPass::FinalBlitPass()
    {
        m_finalBlitMat = Material::CreateFromShader("shaders/final_blit.shader");
        m_finalBlitMat->depthMode = DepthMode::ALWAYS;
        m_finalBlitMat->depthWrite = false;
        m_lutTexture = Image::LoadFromFile("textures/testLut.png");
    }

    void FinalBlitPass::Execute()
    {
        ZoneScoped;
        
        if(m_finalBlitMat == nullptr)
        {
            return;
        }

        m_finalBlitMat->Set(MIN_LUMINANCE, m_minLuminance);
        m_finalBlitMat->Set(MAX_LUMINANCE, m_maxLuminance);
        m_finalBlitMat->SetTexture(LUT_TEX, m_lutTexture);

        RenderingUtils::Blit(GetRC()->shadingBufferTex.lock(), nullptr, m_finalBlitMat.get());
    }

    void FinalBlitPass::DrawConsoleUi()
    {
        // ImGui::SliderFloat("Min Lum Intensity", &m_minLuminance, 0.0f, 10.0f);
        // ImGui::SliderFloat("Max Lum Intensity", &m_maxLuminance, 0.0f, 10.0f);
    }
}
