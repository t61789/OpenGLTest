#include "render_context.h"

#include "material.h"
#include "game_resource.h"
#include "utils.h"

#include "render_texture.h"
#include "shared_object.h"
#include "objects/camera_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    RenderContext::~RenderContext()
    {
        for (auto it : m_rts)
        {
            DECREF(it.second);
        }
    }

    void RenderContext::SetViewProjMatrix(const CameraComp* cam)
    {
        auto cameraLocalToWorld = cam->owner->transform->GetLocalToWorld();
        cameraLocalToWorld[0][2] = -cameraLocalToWorld[0][2];
        cameraLocalToWorld[1][2] = -cameraLocalToWorld[1][2];
        cameraLocalToWorld[2][2] = -cameraLocalToWorld[2][2];
        auto viewMatrix = cameraLocalToWorld.Inverse();

        auto aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
        auto projectionMatrix = create_projection(cam->fov, aspect, cam->nearClip, cam->farClip);

        SetViewProjMatrix(viewMatrix, projectionMatrix);
    }

    void RenderContext::SetViewProjMatrix(const Matrix4x4& view, const Matrix4x4& proj)
    {
        vMatrix = view;
        pMatrix = proj;
        vpMatrix = proj * view;
        
        auto perViewMaterial = GameResource::Ins()->GetPredefinedMaterial(PER_VIEW_CBUFFER);
        perViewMaterial->Set(VP, vpMatrix);
    }

    void RenderContext::RegisterRt(RenderTexture* rt)
    {
        auto it = m_rts.find(rt->desc.name);
        if (it != m_rts.end())
        {
            DECREF(it->second);
        }
        m_rts[rt->desc.name] = rt;
        INCREF(rt);
    }

    void RenderContext::UnRegisterRt(const RenderTexture* rt)
    {
        auto it = m_rts.find(rt->desc.name);
        if (it != m_rts.end())
        {
            DECREF(it->second);
            m_rts.erase(it);
        }
    }

    RenderTexture* RenderContext::GetRt(const std::string& name)
    {
        auto it = m_rts.find(name);
        if (it != m_rts.end())
        {
            return it->second;
        }
        return nullptr;
    }
}