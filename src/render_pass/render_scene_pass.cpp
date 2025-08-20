#include "render_scene_pass.h"

#include <tracy/Tracy.hpp>

#include "scene.h"

#include "render_target.h"
#include "rendering_utils.h"
#include "objects/light_comp.h"

namespace op
{
    RenderScenePass::RenderScenePass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        ZoneScoped;
        
        // glGenBuffers(1, &m_buffer);
        // glBindBuffer(GL_UNIFORM_BUFFER, m_buffer);
        // float data[8] = {1,2,3,4,5,6,7,8};
        // glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 8, data, GL_STATIC_DRAW);
    }

    RenderScenePass::~RenderScenePass()
    {
        // glDeleteBuffers(1, &m_buffer);
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

        GET_GLOBAL_CBUFFER->Set(EXPOSURE_MULTIPLIER, scene->tonemappingExposureMultiplier); // TODO
    
        RenderingUtils::RenderScene(m_renderContext->visibleRenderObjs);
    }
}