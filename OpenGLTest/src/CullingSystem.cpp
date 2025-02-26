#include "CullingSystem.h"

#include <array>

#include "Bounds.h"

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

using namespace glm;
using namespace std;

CullingSystem::CullingSystem(RenderContext* renderContext)
{
    m_renderContext = renderContext;
}

void CullingSystem::Cull()
{
    // 把visibleRenderObjs扔了，剔除allRenderObjs生成一个新的
    
    delete m_renderContext->visibleRenderObjs;
    auto result = new vector<Entity*>();
    
    const auto& renderObjs = *m_renderContext->allRenderObjs;
    auto boundsWS = GetWorldSpaceAABB(renderObjs);
    m_bounds = boundsWS;
    auto planes = GetFrustumPlanes(m_renderContext->vpMatrix);

    for (int i = 0; i < boundsWS.size(); ++i)
    {
        if (CullOnce(boundsWS[i], planes))
        {
            result->push_back(renderObjs[i]);
        }
    }

    m_renderContext->visibleRenderObjs = result;
}

void CullingSystem::OnDrawConsoleGui()
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

bool CullingSystem::CullOnce(const Bounds& bounds, const array<vec4, 6>& planes)
{
    return FrustumCull(bounds, planes);
}

vector<Bounds> CullingSystem::GetWorldSpaceAABB(const vector<Entity*>& renderObjs)
{
    auto result = vector<Bounds>(renderObjs.size());

    for (int i = 0; i < result.size(); ++i)
    {
        auto obj = renderObjs[i];
        auto m = obj->GetLocalToWorld();
        auto boundsOS = obj->bounds;
        auto centerWS = vec3(m * vec4(boundsOS.center, 1));
        vec3 extentsWS;
        extentsWS.x = dot(boundsOS.extents, abs(vec3(m[0])));
        extentsWS.y = dot(boundsOS.extents, abs(vec3(m[1])));
        extentsWS.z = dot(boundsOS.extents, abs(vec3(m[2])));
        auto boundsWS = Bounds(centerWS, extentsWS);
        result[i] = boundsWS;
    }

    return result;
}

array<vec4, 6> CullingSystem::GetFrustumPlanes(const mat4& vp)
{
    auto tvp = transpose(vp);
    array<vec4, 6> planes;

    // 法线指向内部，w为沿法线方向的偏移值
    planes[0] = tvp[3] + tvp[0]; // left
    planes[1] = tvp[3] - tvp[0]; // right
    planes[2] = tvp[3] + tvp[1]; // bottom
    planes[3] = tvp[3] - tvp[1]; // top
    planes[4] = tvp[3] + tvp[2]; // near
    planes[5] = tvp[3] - tvp[2]; // far

    for (auto& plane : planes)
    {
        auto len = length(vec3(plane));
        plane.x /= len;
        plane.y /= len;
        plane.z /= len;
        plane.w /= -len;
    }

    return planes;
}

bool CullingSystem::FrustumCull(const Bounds& bounds, const std::array<vec4, 6>& planes)
{
     bool result = true;
     for (uint i = 0; i < 6; i++)
     {
         auto plane = planes[i];
         auto normal = vec3(plane);
         vec3 s = sign(normal);
         vec3 offset = bounds.extents * s;
         // 只要有任何一个面，包围盒完全在这个面外面，则包围盒在视锥体外面
         result = result && (
             dot(bounds.center + offset, normal) >= plane.w ||
             dot(bounds.center - offset, normal) >= plane.w);
     }

     return result;
}
