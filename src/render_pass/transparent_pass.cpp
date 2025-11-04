#include "transparent_pass.h"

#include "rendering_utils.h"
#include "render_context.h"
#include "scene.h"
#include "objects/render_comp.h"

namespace op
{
    void TransparentPass::Execute()
    {
        auto shadingBufferTex = GetRC()->shadingBufferTex.lock();
        auto depthBufferTex = GetRC()->depthBufferTex.lock();
        assert(shadingBufferTex && depthBufferTex);
        auto usingRenderTarget = RenderTarget::Using({shadingBufferTex, depthBufferTex});

        assert(GetRC()->scene);
        assert(GetRC()->scene->GetIndices()->GetTransparentSortJob());
        GetRC()->scene->GetIndices()->GetTransparentSortJob()->WaitForStop();

        for (const auto& rc : GetRC()->scene->GetIndices()->GetTransparentRenderComps())
        {
            assert(!rc.expired());
            auto renderComp = rc.lock();
            RenderingUtils::RenderMesh({
                renderComp->GetMesh().get(),
                renderComp->GetMaterial().get(),
                renderComp->HasOddNegativeScale(),
                renderComp->GetObjectIndex()
            });
        }
    }
}
