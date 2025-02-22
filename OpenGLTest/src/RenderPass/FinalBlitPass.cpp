#include "FinalBlitPass.h"

#include "RenderingUtils.h"

FinalBlitPass::FinalBlitPass()
{
    finalBlitMat = Material::CreateEmptyMaterial("shaders/final_blit.glsl");
    finalBlitMat->IncRef();
    auto desc = ImageDescriptor::GetDefault();
    desc.needFlipVertical = false;
    lutTexture = Image::LoadFromFile("textures/testLut.png", desc);
    lutTexture->IncRef();
}

FinalBlitPass::~FinalBlitPass()
{
    finalBlitMat->DecRef();
    lutTexture->DecRef();
}

std::string FinalBlitPass::GetName()
{
    return "Final Blit Pass";
}

void FinalBlitPass::Execute(RenderContext& renderContext)
{
    RenderTarget::ClearFrameBuffer(0, glm::vec4(0), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderTarget::UseScreenTarget();
    
    if(renderContext.quadMesh == nullptr || finalBlitMat == nullptr)
    {
        return;
    }

    finalBlitMat->SetTextureValue("_LutTex", lutTexture);
    RenderingUtils::RenderMesh(renderContext, renderContext.quadMesh, finalBlitMat, glm::mat4()); 
}
