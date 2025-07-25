#include "main_light_shadow_pass.h"

#include "scene.h"
#include "render_target.h"
#include "material.h"
#include "render_texture.h"
#include "shared_object.h"
#include "rendering_utils.h"
#include "utils.h"
#include "objects/camera_comp.h"
#include "objects/light_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    MainLightShadowPass::MainLightShadowPass(RenderContext* renderContext) : RenderPass(renderContext)
    {
        m_drawShadowMat = Material::CreateEmptyMaterial("shaders/draw_shadow.glsl");
        INCREF(m_drawShadowMat);
    }

    MainLightShadowPass::~MainLightShadowPass()
    {
        if (m_mainLightShadowRt)
        {
            m_renderContext->UnRegisterRt(m_mainLightShadowRt);
            DECREF(m_mainLightShadowRt);
        }
        DECREF(m_drawShadowMat);
    }

    std::string MainLightShadowPass::GetName()
    {
        return "Main Light Shadow Pass";
    }

    void MainLightShadowPass::Execute()
    {
        UpdateRt();

        auto camera = m_renderContext->camera;
        auto scene = m_renderContext->scene;
        if(camera == nullptr || scene == nullptr)
        {
            return;
        }
    
        auto lightDirection = -Vec3::One().Normalize();
        if (m_renderContext->mainLight)
        {
            lightDirection = m_renderContext->mainLight->owner->transform->GetLocalToWorld().Forward().Normalize();
        }

        constexpr float range = 30;
        float range2 = 20;
        auto distancePerTexel = range * 2 / static_cast<float>(m_renderContext->mainLightShadowSize);
        // 计算阴影矩阵
        Vec3 forward, right, up;
        forward = lightDirection;
        gram_schmidt_ortho(&forward.x, &Vec3::Up().x, &right.x, &up.x);
        auto shadowCameraToWorld = Matrix4x4(
            right.x, up.x, forward.x, 0,
            right.y, up.y, forward.y, 0,
            forward.x, forward.y, forward.z, 0,
            0, 0, 0, 1);
        auto worldToShadowCamera = shadowCameraToWorld.Inverse();
        // 希望以摄像机为中心，但是先把摄像机位置转到阴影空间，然后对齐每个纹素，避免阴影光栅化时闪烁
        auto cameraPositionVS = worldToShadowCamera * Vec4(camera->owner->transform->GetPosition(), 1);
        cameraPositionVS.x = std::floor(cameraPositionVS.x / distancePerTexel) * distancePerTexel;
        cameraPositionVS.y = std::floor(cameraPositionVS.y / distancePerTexel) * distancePerTexel;
        auto alignedCameraPositionWS = (shadowCameraToWorld * cameraPositionVS).ToVec3();
        // 得到对齐后的摄像机位置
        auto shadowCameraPositionWS = alignedCameraPositionWS + forward * range2;
        // 把阴影矩阵的中心设置为对齐后的摄像机位置
        shadowCameraToWorld[0][3] = shadowCameraPositionWS.x; // 第3列第0行
        shadowCameraToWorld[1][3] = shadowCameraPositionWS.y;
        shadowCameraToWorld[2][3] = shadowCameraPositionWS.z;
        worldToShadowCamera = shadowCameraToWorld.Inverse();
    
        auto projMatrix = Utils::CreateOrthoProjection(range, -range, range, -range, 2 * range2, 0.05f);
        m_renderContext->SetViewProjMatrix(worldToShadowCamera, projMatrix); // TODO push

        Material::SetGlobalMat4Value("_MainLightShadowVP", projMatrix * worldToShadowCamera);

        auto renderTarget = RenderTarget::Get(nullptr, m_mainLightShadowRt);
        renderTarget->Clear(1.0f);
        renderTarget->Use();

        m_renderContext->replaceMaterial = m_drawShadowMat;
        RenderingUtils::RenderScene(*m_renderContext, m_renderContext->allRenderObjs);
        m_renderContext->replaceMaterial = nullptr;

        // 准备绘制参数
        m_renderContext->SetViewProjMatrix(camera);
    }

    void MainLightShadowPass::UpdateRt()
    {
        if (m_mainLightShadowRt == nullptr)
        {
            m_mainLightShadowRt = new RenderTexture(
                RenderTextureDescriptor(
                    m_renderContext->mainLightShadowSize,
                    m_renderContext->mainLightShadowSize,
                    Depth,
                    Point,
                    Clamp,
                    "_MainLightShadowMapTex"));
            INCREF(m_mainLightShadowRt);
            m_renderContext->RegisterRt(m_mainLightShadowRt);
            Material::SetGlobalTextureValue("_MainLightShadowMapTex", m_mainLightShadowRt);
        }
        m_mainLightShadowRt->Resize(m_renderContext->mainLightShadowSize, m_renderContext->mainLightShadowSize);
    }
}
