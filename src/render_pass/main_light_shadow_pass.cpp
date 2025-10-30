#include "main_light_shadow_pass.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "scene.h"

#include "material.h"
#include "render_texture.h"
#include "rendering_utils.h"
#include "render_context.h"
#include "utils.h"
#include "objects/camera_comp.h"
#include "objects/light_comp.h"
#include "objects/transform_comp.h"
#include "render/render_target.h"
#include "render/render_target_pool.h"
#include "render/gl/gl_texture.h"

namespace op
{
    MainLightShadowPass::MainLightShadowPass() : IRenderPass()
    {
        m_drawShadowMat = Material::CreateFromShader("shaders/draw_shadow.");
    }

    MainLightShadowPass::~MainLightShadowPass()
    {
    }

    std::string MainLightShadowPass::GetName()
    {
        return "Main Light Shadow Pass";
    }

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

        Matrix4x4 shadowViewMatrix, shadowProjMatrix;
        Vec3 shadowCenter;
        CalcShadowMatrix(
            GetRC()->mainLight->GetOwner()->transform,
            camera->GetOwner()->transform,
            shadowViewMatrix,
            shadowProjMatrix,
            shadowCenter);
    
        GetGlobalCbuffer()->Set(MAINLIGHT_SHADOW_VP, shadowProjMatrix * shadowViewMatrix);

        {
            // UsingRenderTarget usingRenderTarget(m_mainLightShadowRt);
            // auto usingShadowVPMatrix = GetRC()->UsingViewProjMatrix(shadowViewMatrix, shadowProjMatrix, shadowCenter);
            //
            // usingRenderTarget.Get()->Clear(1.0f);
            //
            // // m_renderContext->replaceMaterial = m_drawShadowMat; // TODO
            // RenderingUtils::RenderScene(*GetRC()->allRenderObjs);
            // // m_renderContext->replaceMaterial = nullptr; TODO
            //
            // // 准备绘制参数
            // m_renderContext->SetViewProjMatrix(camera);
        }
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

    void MainLightShadowPass::CalcShadowMatrix(
        TransformComp* lightTrans,
        TransformComp* cameraTrans,
        Matrix4x4& viewMatrix,
        Matrix4x4& projMatrix,
        Vec3& shadowCenter)
    {
        auto lightForward = lightTrans ? lightTrans->GetLocalToWorld().Forward() : -Vec3::One().Normalize();
        auto cameraPos = cameraTrans ? cameraTrans->GetPosition() : Vec3::Zero();
        
        constexpr float range = 30;
        float range2 = 20;
        auto distancePerTexel = range * 2 / static_cast<float>(GetRC()->mainLightShadowSize);
        // 计算阴影矩阵
        Vec3 forward, right, up;
        forward = lightForward;
        gram_schmidt_ortho(&forward.x, &Vec3::Up().x, &right.x, &up.x);
        auto shadowCameraToWorld = Matrix4x4(
            right.x, up.x, forward.x, 0,
            right.y, up.y, forward.y, 0,
            forward.x, forward.y, forward.z, 0,
            0, 0, 0, 1);
        viewMatrix = shadowCameraToWorld.Inverse();
        // 希望以摄像机为中心，但是先把摄像机位置转到阴影空间，然后对齐每个纹素，避免阴影光栅化时闪烁
        auto cameraPositionVS = viewMatrix * Vec4(cameraPos, 1);
        cameraPositionVS.x = std::floor(cameraPositionVS.x / distancePerTexel) * distancePerTexel;
        cameraPositionVS.y = std::floor(cameraPositionVS.y / distancePerTexel) * distancePerTexel;
        auto alignedCameraPositionWS = (shadowCameraToWorld * cameraPositionVS).ToVec3();
        // 得到对齐后的摄像机位置
        shadowCenter = alignedCameraPositionWS + forward * range2;
        // 把阴影矩阵的中心设置为对齐后的摄像机位置
        shadowCameraToWorld[0][3] = shadowCenter.x; // 第3列第0行
        shadowCameraToWorld[1][3] = shadowCenter.y;
        shadowCameraToWorld[2][3] = shadowCenter.z;
        viewMatrix = shadowCameraToWorld.Inverse();

        projMatrix = create_ortho_projection(range, -range, range, -range, 2 * range2, 0.05f);
    }
}
