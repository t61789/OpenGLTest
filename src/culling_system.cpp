#include "culling_system.h"

#include <array>

#include <tracy/Tracy.hpp>

#include "render_context.h"
#include "bounds.h"
#include "object.h"
#include "scene.h"
#include "objects/batch_render_comp.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"

// bool FrustumCulling(const float3 center, const float3 extents)
// {
//     bool result = true;
//     for (uint i = 0; i < 6; i++)
//     {
//         float4 plane = _Planes[i];
//         float3 normal = plane.xyz;
//         float3 s = sign(normal);
//         float3 offset = extents * s;
//         result = result && (
//             dot(center + offset, normal) >= plane.w ||
//             dot(center - offset, normal) >= plane.w);
//     }
//
//     return result;
// }

namespace op
{
    using namespace std;

    void CullingSystem::Cull()
    {
        ZoneScoped;
        
        // 把visibleRenderObjs扔了，剔除allRenderObjs生成一个新的
        GetRC()->visibleRenderObjs.clear();
    
        const auto& renderObjs = *GetRC()->allRenderObjs;
        auto planes = get_frustum_planes(GetRC()->vpMatrix);

        for (const auto& renderObj : renderObjs)
        {
            // if (CullOnce(renderObj->GetWorldBounds(), planes))
            // {
                GetRC()->visibleRenderObjs.push_back(renderObj.lock().get());
            // }
        }

        for (const auto& batchRenderCompPtr : GetRC()->scene->GetIndices()->GetCompStorage()->GetComps<BatchRenderComp>())
        {
            if (batchRenderCompPtr.expired())
            {
                continue;
            }
            auto batchRenderComp = batchRenderCompPtr.lock();

            auto inView = batchRenderComp->IsEnable() && CullOnce(batchRenderComp->GetWorldBounds(), planes);
            batchRenderComp->SetInView(inView);
        }
    }

    void CullingSystem::DrawConsoleUi()
    {
        // for (auto bounds : m_bounds)
        // {
        //     Utils::DebugDrawCube(
        //         bounds,
        //         m_renderContext->vMatrix,
        //         m_renderContext->pMatrix,
        //         vec2(m_renderContext->screenWidth, m_renderContext->screenHeight));
        // }
    }

    bool CullingSystem::CullOnce(const Bounds& bounds, const array<Vec4, 6>& planes)
    {
        return frustum_culling(planes, bounds.center, bounds.extents);
    }
}
