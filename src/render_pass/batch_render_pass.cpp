#include "batch_render_pass.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "render_context.h"
#include "render/batch_render_unit.h"
#include "render/render_target_pool.h"

namespace op
{
    void BatchRenderPass::Execute()
    {
        ZoneScoped;
        
        auto usingGBufferRenderTarget = GetRC()->UsingGBufferRenderTarget();

        GetGR()->GetBatchRenderUnit()->Execute();
    }
}
