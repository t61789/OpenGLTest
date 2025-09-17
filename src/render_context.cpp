#include "render_context.h"

#include "material.h"
#include "game_resource.h"

#include "render_texture.h"
#include "objects/camera_comp.h"
#include "objects/transform_comp.h"
#include "render/render_target_pool.h"

namespace op
{
    void RenderContext::SetViewProjMatrix(const CameraComp* cam)
    {
        auto cameraLocalToWorld = cam->GetOwner()->transform->GetLocalToWorld();
        cameraLocalToWorld[0][2] = -cameraLocalToWorld[0][2];
        cameraLocalToWorld[1][2] = -cameraLocalToWorld[1][2];
        cameraLocalToWorld[2][2] = -cameraLocalToWorld[2][2];
        auto viewMatrix = cameraLocalToWorld.Inverse();

        auto aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
        auto projectionMatrix = create_projection(cam->fov, aspect, cam->nearClip, cam->farClip);

        SetViewProjMatrix(viewMatrix, projectionMatrix, cam->GetOwner()->transform->GetPosition());
    }

    void RenderContext::SetViewProjMatrix(const Matrix4x4& view, const Matrix4x4& proj)
    {
        auto invV = view.Inverse();
        Vec3 cameraPos = {
            invV[0][3],
            invV[1][3],
            invV[2][3]
        };

        SetViewProjMatrix(view, proj, cameraPos);
    }

    void RenderContext::SetViewProjMatrix(const Matrix4x4& view, const Matrix4x4& proj, const Vec3& cameraPos)
    {
        vMatrix = view;
        pMatrix = proj;
        vpMatrix = proj * view;
        
        auto perViewCbuffer = GetPerViewCbuffer();
        perViewCbuffer->Set(VP, vpMatrix);
        perViewCbuffer->Set(CAMERA_POSITION_WS, Vec4(cameraPos, 0.0f));
    }

    UsingRenderTarget RenderContext::UsingGBufferRenderTarget()
    {
        static vecsp<RenderTexture> rts;
        rts.clear();
        for (auto& rt : gBufferTextures)
        {
            rts.push_back(rt.lock());
        }
        
        return UsingRenderTarget(rts);
    }
}
