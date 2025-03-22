#include "RenderScenePass.h"

#include "Scene.h"
#include "Material.h"
#include "RenderTarget.h"
#include "RenderingUtils.h"
#include "Objects/LightComp.h"

namespace op
{
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

        Material::SetGlobalFloatValue("_ExposureMultiplier", scene->tonemappingExposureMultiplier);
    
        RenderingUtils::RenderScene(*m_renderContext, m_renderContext->visibleRenderObjs);
    }
}