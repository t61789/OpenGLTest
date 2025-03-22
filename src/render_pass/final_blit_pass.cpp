#include "FinalBlitPass.h"

#include "imgui.h"

#include "RenderTarget.h"
#include "Material.h"
#include "BuiltInRes.h"
#include "RenderingUtils.h"

namespace op
{
    FinalBlitPass::FinalBlitPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        finalBlitMat = Material::CreateEmptyMaterial("shaders/final_blit.glsl");
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
        auto quad = BuiltInRes::GetInstance()->quadMesh;
        if(finalBlitMat == nullptr)
        {
            return;
        }
    
        RenderTarget::ClearFrameBuffer(0, glm::vec4(0), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderTarget::UseScreenTarget();

        finalBlitMat->SetFloatValue("_MinLuminance", m_minLuminance);
        finalBlitMat->SetFloatValue("_MaxLuminance", m_maxLuminance);

        finalBlitMat->SetTextureValue("_LutTex", lutTexture);
        RenderingUtils::RenderMesh(*m_renderContext, quad, finalBlitMat, glm::mat4());
    }

    void FinalBlitPass::DrawConsoleUi()
    {
        // ImGui::SliderFloat("Min Lum Intensity", &m_minLuminance, 0.0f, 10.0f);
        // ImGui::SliderFloat("Max Lum Intensity", &m_maxLuminance, 0.0f, 10.0f);
    }
}
