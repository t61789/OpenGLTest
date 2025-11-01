#include "main_light_shadow_pass.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "gui.h"
#include "scene.h"

#include "material.h"
#include "render_texture.h"
#include "render_context.h"
#include "utils.h"
#include "objects/camera_comp.h"
#include "objects/transform_comp.h"
#include "render/render_target.h"
#include "render/render_target_pool.h"
#include "render/gl/gl_texture.h"

namespace op
{
    void MainLightShadowPass::Execute()
    {
        ZoneScoped;
        
        UpdateRt();

        auto camera = GetRC()->camera;
        auto scene = GetRC()->scene;
        if(camera == nullptr || scene == nullptr)
        {
            return;
        }

        auto shadowViewProj = GetRC()->shadowVPInfo;
        GetGlobalCbuffer()->Set(MAINLIGHT_SHADOW_VP, shadowViewProj->vpMatrix);

        DrawUI();

        {
            UsingRenderTarget usingRenderTarget(m_mainLightShadowRt);
            auto usingShadowVPMatrix = GetRC()->UsingViewProjMatrix(shadowViewProj);
            
            usingRenderTarget.Get()->Clear(1.0f);

            GetGR()->GetBatchRenderUnit()->Execute(ViewGroup::SHADOW);
        }
    }

    void MainLightShadowPass::DrawUI()
    {
        auto vpInfo = GetRC()->CurViewProjMatrix();
        auto shadowVpInfo = GetRC()->shadowVPInfo;
        auto shadowLocalToWorld = shadowVpInfo->vMatrix.Inverse();
        
        Gui::Ins()->DebugDrawLine(
            shadowLocalToWorld.Position(),
            shadowLocalToWorld.Position() + shadowLocalToWorld.OriginRight(),
            vpInfo->vMatrix,
            vpInfo->pMatrix,
            Vec2(GetRC()->screenWidth, GetRC()->screenHeight),
            IM_COL32(255, 0, 0, 255));
        
        Gui::Ins()->DebugDrawLine(
            shadowLocalToWorld.Position(),
            shadowLocalToWorld.Position() + shadowLocalToWorld.OriginUp(),
            vpInfo->vMatrix,
            vpInfo->pMatrix,
            Vec2(GetRC()->screenWidth, GetRC()->screenHeight),
            IM_COL32(0, 255, 0, 255));
        
        Gui::Ins()->DebugDrawLine(
            shadowLocalToWorld.Position(),
            shadowLocalToWorld.Position() + shadowLocalToWorld.OriginForward(),
            vpInfo->vMatrix,
            vpInfo->pMatrix,
            Vec2(GetRC()->screenWidth, GetRC()->screenHeight),
            IM_COL32(0, 0, 255, 255));
    }

    void MainLightShadowPass::UpdateRt()
    {
        if (m_mainLightShadowRt == nullptr)
        {
            m_mainLightShadowRt = msp<RenderTexture>(RtDesc{
                "_MainLightShadowTex",
                GetRC()->mainLightShadowSize,
                GetRC()->mainLightShadowSize,
                TextureFormat::DEPTH,
                TextureFilterMode::POINT,
                TextureWrapMode::CLAMP});
            GetGlobalTextureSet()->SetTexture(MAIN_LIGHT_SHADOW_MAP_TEX, m_mainLightShadowRt);
        }
        m_mainLightShadowRt->Resize(GetRC()->mainLightShadowSize, GetRC()->mainLightShadowSize);
    }
}
