#include "render_scene_pass.h"

#include <tracy/Tracy.hpp>

#include "scene.h"
#include "material.h"
#include "render_target.h"
#include "rendering_utils.h"
#include "objects/light_comp.h"

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
        ZoneScoped;
        
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