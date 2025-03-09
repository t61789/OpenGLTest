#include "PreparingPass.h"

#include "RenderTarget.h"
#include "Material.h"
#include "Objects/CameraComp.h"

PreparingPass::PreparingPass(RenderContext* renderContext) : RenderPass(renderContext)
{
}

std::string PreparingPass::GetName()
{
    return "Preparing Pass";
}

void PreparingPass::Execute()
{
    if(m_renderContext->camera == nullptr)
    {
        return;
    }

    Material::SetGlobalVector4Value("_CameraPositionWS", glm::vec4(m_renderContext->camera->owner->position, 0));

    std::vector<glm::vec4> clearColors = {
        glm::vec4(0.5f),
        glm::vec4(0.0f),
        glm::vec4(1.0f),
    };
    RenderTarget::Get(*m_renderContext->gBufferDesc)->Clear(clearColors, 1.0f);

    auto viewportSize = glm::vec4(m_renderContext->screenWidth, m_renderContext->screenHeight, 0, 0);
    Material::SetGlobalVector4Value("_ViewportSize", viewportSize);
}

void PreparingPass::PrepareLightInfos()
{
}
