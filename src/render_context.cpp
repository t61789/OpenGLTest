#include "render_context.h"

#include "material.h"
#include "game_resource.h"

#include "render_texture.h"
#include "objects/camera_comp.h"
#include "render/render_target_pool.h"

namespace op
{
    void ViewProjInfo::UpdateFrustumPlanes()
    {
        frustumPlanes = get_frustum_planes(vpMatrix);
    }

    void ViewProjInfo::UpdateIVP()
    {
        ivpMatrix = vpMatrix.Inverse();
    }

    sp<ViewProjInfo> ViewProjInfo::Create(cr<Matrix4x4> vMatrix, cr<Matrix4x4> pMatrix, const bool useIVP)
    {
        auto invV = vMatrix.Inverse();
        Vec3 viewCenter = {
            invV[0][3],
            invV[1][3],
            invV[2][3]
        };

        return Create(vMatrix, pMatrix, viewCenter, useIVP);
    }

    sp<ViewProjInfo> ViewProjInfo::Create(cr<Matrix4x4> vMatrix, cr<Matrix4x4> pMatrix, cr<Vec3> viewCenter, const bool useIVP)
    {
        auto info = msp<ViewProjInfo>();
        info->vMatrix = vMatrix;
        info->pMatrix = pMatrix;
        info->vpMatrix = pMatrix * vMatrix;
        info->viewCenter = viewCenter;
        if (useIVP)
        {
            info->ivpMatrix = info->vpMatrix.Inverse();
        }

        return info;
    }

    UsingRenderTarget RenderContext::UsingGBufferRenderTarget()
    {
        vecsp<RenderTexture> rts(gBufferTextures.size());
        for (size_t i = 0; i < gBufferTextures.size(); ++i)
        {
            rts[i] = gBufferTextures[i].lock();
        }
        return UsingRenderTarget(rts);
    }

    void RenderContext::PushViewProjMatrix(crsp<ViewProjInfo> viewProjInfo)
    {
        m_vpMatrixStack.push_back(viewProjInfo);

        SetViewProjMatrix(viewProjInfo);
    }

    void RenderContext::PopViewProjMatrix()
    {
        if (m_vpMatrixStack.empty())
        {
            return;
        }
        
        this->m_vpMatrixStack.pop_back();
        if (!m_vpMatrixStack.empty())
        {
            SetViewProjMatrix(m_vpMatrixStack.back());
        }
    }

    UsingObject RenderContext::UsingViewProjMatrix(crsp<ViewProjInfo> viewProjInfo)
    {
        PushViewProjMatrix(viewProjInfo);

        return UsingObject([this]
        {
            this->PopViewProjMatrix();
        });
    }

    crsp<ViewProjInfo> RenderContext::CurViewProjMatrix() const
    {
        assert(!m_vpMatrixStack.empty());

        return m_vpMatrixStack.back();
    }

    void RenderContext::SetViewProjMatrix(crsp<ViewProjInfo> viewProjInfo)
    {
        auto perViewCbuffer = GetPerViewCbuffer();
        perViewCbuffer->Set(VP, viewProjInfo->vpMatrix);
        perViewCbuffer->Set(CAMERA_POSITION_WS, Vec4(viewProjInfo->viewCenter, 0.0f));

        if (viewProjInfo->ivpMatrix.has_value())
        {
            perViewCbuffer->Set(IVP, viewProjInfo->ivpMatrix);
        }
    }
}
