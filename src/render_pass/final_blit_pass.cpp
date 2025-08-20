#include "final_blit_pass.h"

#include <tracy/Tracy.hpp>

#include "imgui.h"

#include "render_target.h"
#include "built_in_res.h"
#include "rendering_utils.h"

namespace op
{
    FinalBlitPass::FinalBlitPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        finalBlitMat = Material::CreateFromShader("shaders/final_blit.shader");
        INCREF(finalBlitMat);
        auto desc = ImageDescriptor::GetDefault();
        desc.needFlipVertical = false;
        lutTexture = Image::LoadFromFile("textures/testLut.png", desc);
        INCREF(lutTexture);
    }

    FinalBlitPass::~FinalBlitPass()
    {
        DECREF(finalBlitMat);
        DECREF(lutTexture);
    }

    std::string FinalBlitPass::GetName()
    {
        return "Final Blit Pass";
    }

    void FinalBlitPass::Execute()
    {
        ZoneScoped;
        
        if(finalBlitMat == nullptr)
        {
            return;
        }
    
        RenderTarget::ClearFrameBuffer(0, Vec4::Zero(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderTarget::UseScreenTarget();

        finalBlitMat->Set(MIN_LUMINANCE, m_minLuminance);
        finalBlitMat->Set(MAX_LUMINANCE, m_maxLuminance);
        finalBlitMat->Set(LUT_TEX, lutTexture);

        auto quad = BuiltInRes::Ins()->quadMesh;
        
        RenderingUtils::RenderMesh(quad, finalBlitMat, Matrix4x4::Identity());
    }

    void FinalBlitPass::DrawConsoleUi()
    {
        // ImGui::SliderFloat("Min Lum Intensity", &m_minLuminance, 0.0f, 10.0f);
        // ImGui::SliderFloat("Max Lum Intensity", &m_maxLuminance, 0.0f, 10.0f);
    }
}
