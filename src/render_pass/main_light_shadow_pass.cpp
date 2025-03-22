#include "MainLightShadowPass.h"

#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"

#include "Scene.h"
#include "RenderTarget.h"
#include "Material.h"
#include "RenderTexture.h"
#include "SharedObject.h"
#include "RenderingUtils.h"
#include "Utils.h"
#include "Objects/CameraComp.h"
#include "Objects/LightComp.h"
#include "Objects/TransformComp.h"

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
    
        auto lightDirection = normalize(glm::vec3(1,1,1));
        if (m_renderContext->mainLight)
        {
            lightDirection = -m_renderContext->mainLight->owner->transform->Forward();
        }

        constexpr float range = 100;
        float range2 = 50;
        auto distancePerTexel = range * 2 / static_cast<float>(m_renderContext->mainLightShadowSize);
        // 计算阴影矩阵
        auto forward = normalize(lightDirection);
        auto right = normalize(cross(glm::vec3(0, 1, 0), forward));
        auto up = normalize(glm::cross(forward, right)); // 右手从x绕到y
        auto shadowCameraToWorld = glm::mat4(
            right.x, right.y, right.z, 0, // 第一列
            up.z, up.y, up.z, 0,
            forward.x, forward.y, forward.z, 0,
            0, 0, 0, 1); // 留空，之后设置
        auto worldToShadowCamera = inverse(shadowCameraToWorld);
        // 希望以摄像机为中心，但是先把摄像机位置转到阴影空间，然后对齐每个纹素，避免阴影光栅化时闪烁
        auto cameraPositionVS = worldToShadowCamera * glm::vec4(camera->owner->transform->GetPosition(), 1);
        cameraPositionVS.x = std::floor(cameraPositionVS.x / distancePerTexel) * distancePerTexel;
        cameraPositionVS.y = std::floor(cameraPositionVS.y / distancePerTexel) * distancePerTexel;
        auto alignedCameraPositionWS = static_cast<glm::vec3>(shadowCameraToWorld * cameraPositionVS);
        // 得到对齐后的摄像机位置
        auto shadowCameraPositionWS = alignedCameraPositionWS + forward * range2;
        // 把阴影矩阵的中心设置为对齐后的摄像机位置
        shadowCameraToWorld[3][0] = shadowCameraPositionWS.x; // 第3列第0行
        shadowCameraToWorld[3][1] = shadowCameraPositionWS.y;
        shadowCameraToWorld[3][2] = shadowCameraPositionWS.z;
        worldToShadowCamera = inverse(shadowCameraToWorld);
    
        auto projMatrix = glm::ortho(-range, range, -range, range, 0.05f, 2 * range2);
        m_renderContext->SetViewProjMatrix(worldToShadowCamera, projMatrix);

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
