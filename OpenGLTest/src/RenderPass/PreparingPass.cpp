#include "PreparingPass.h"

std::string PreparingPass::GetName()
{
    return "Preparing Pass";
}

void PreparingPass::Execute(RenderContext& renderContext)
{
    if(renderContext.camera == nullptr)
    {
        return;
    }

    Material::SetGlobalVector4Value("_CameraPositionWS", glm::vec4(renderContext.camera->position, 0));
    
    renderContext.cameraPositionWS = renderContext.camera->position;

    std::vector<glm::vec4> clearColors = {
        glm::vec4(0.5f),
        glm::vec4(0.0f),
        glm::vec4(1.0f),
    };
    RenderTarget::Get(*renderContext.gBufferDesc)->Clear(clearColors, 1.0f);
}
