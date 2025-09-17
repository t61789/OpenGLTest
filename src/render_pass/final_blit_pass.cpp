#include "final_blit_pass.h"

#include <tracy/Tracy.hpp>

#include "built_in_res.h"
#include "image.h"
#include "material.h"
#include "rendering_utils.h"
#include "render/render_target.h"
#include "render/render_target_pool.h"

namespace op
{
    FinalBlitPass::FinalBlitPass()
    {
        m_finalBlitMat = Material::CreateFromShader("shaders/final_blit.shader");
        auto desc = ImageDescriptor::GetDefault();
        desc.needFlipVertical = false;
        m_lutTexture = Image::LoadFromFile("textures/testLut.png", desc);
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
        m_finalBlitMat->Set(LUT_TEX, m_lutTexture);

        RenderingUtils::Blit(nullptr, nullptr, m_finalBlitMat.get()); // TODO depth
    }

    void FinalBlitPass::DrawConsoleUi()
    {
        // ImGui::SliderFloat("Min Lum Intensity", &m_minLuminance, 0.0f, 10.0f);
        // ImGui::SliderFloat("Max Lum Intensity", &m_maxLuminance, 0.0f, 10.0f);
    }
}
