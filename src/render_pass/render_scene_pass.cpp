#include "render_scene_pass.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "scene.h"

#include "rendering_utils.h"
#include "render_context.h"
#include "render/render_target_pool.h"
#include "render/gl/gl_cbuffer.h"

namespace op
{
    void RenderScenePass::Execute()
    {
        ZoneScoped;
        
        auto camera = GetRC()->camera;
        auto scene = GetRC()->scene;
        if(camera == nullptr || scene == nullptr || scene->GetRoot() == nullptr)
        {
            return;
        }

        auto usingGBufferRenderTarget = GetRC()->UsingGBufferRenderTarget();
        
        GetGR()->GetPredefinedCbuffer(GLOBAL_CBUFFER)->Set(EXPOSURE_MULTIPLIER, scene->tonemappingExposureMultiplier); // TODO
    
        RenderingUtils::RenderScene(GetRC()->visibleRenderObjs);
    }
}
