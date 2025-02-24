#include "RenderScenePass.h"

#include "RenderingUtils.h"

RenderScenePass::RenderScenePass(RenderContext* renderContext) : RenderPass(renderContext)
{
}

std::string RenderScenePass::GetName()
{
    return "Render Scene Pass";
}

void RenderScenePass::Execute()
{
    auto camera = m_renderContext->camera;
    auto scene = m_renderContext->scene;
    if(camera == nullptr || scene == nullptr || scene->sceneRoot == nullptr)
    {
        return;
    }

    RenderTarget::Get(*m_renderContext->gBufferDesc)->Use();

    Material::SetGlobalVector4Value("_MainLightDirection", glm::vec4(glm::normalize(scene->mainLightDirection), 0));
    Material::SetGlobalVector4Value("_MainLightColor", glm::vec4(scene->mainLightColor, 0));
    Material::SetGlobalFloatValue("_ExposureMultiplier", scene->tonemappingExposureMultiplier);
    
    RenderingUtils::RenderScene(*m_renderContext);
}
