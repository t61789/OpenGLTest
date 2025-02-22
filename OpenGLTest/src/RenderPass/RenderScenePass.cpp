#include "RenderScenePass.h"

#include "RenderingUtils.h"

std::string RenderScenePass::GetName()
{
    return "Render Scene Pass";
}

void RenderScenePass::Execute(RenderContext& renderContext)
{
    auto camera = renderContext.camera;
    auto scene = renderContext.scene;
    if(camera == nullptr || scene == nullptr || scene->sceneRoot == nullptr)
    {
        return;
    }

    RenderTarget::Get(*renderContext.gBufferDesc)->Use();

    Material::SetGlobalVector4Value("_MainLightDirection", glm::vec4(normalize(scene->mainLightDirection), 0));
    Material::SetGlobalVector4Value("_MainLightColor", glm::vec4(scene->mainLightColor, 0));
    Material::SetGlobalFloatValue("_ExposureMultiplier", scene->tonemappingExposureMultiplier);
    
    RenderingUtils::RenderScene(renderContext);
}
