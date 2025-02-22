#include "MainLightShadowPass.h"

#include <ext/matrix_clip_space.hpp>
#include "glm.hpp"
#include "RenderingUtils.h"

MainLightShadowPass::MainLightShadowPass()
{
    m_drawShadowMat = Material::CreateEmptyMaterial("shaders/draw_shadow.glsl");
    m_drawShadowMat->IncRef();
}

MainLightShadowPass::~MainLightShadowPass()
{
    m_mainLightShadowRt->DecRef();
    m_drawShadowMat->DecRef();
}

std::string MainLightShadowPass::GetName()
{
    return "Main Light Shadow Pass";
}

void MainLightShadowPass::Execute(RenderContext& renderContext)
{
    UpdateRt(renderContext);

    auto camera = renderContext.camera;
    auto scene = renderContext.scene;
    if(camera == nullptr || scene == nullptr)
    {
        return;
    }

    constexpr float range = 10;
    float range2 = 50;
    auto distancePerTexel = range * 2 / static_cast<float>(renderContext.mainLightShadowSize);
    // 计算阴影矩阵
    auto forward = glm::normalize(scene->mainLightDirection);
    auto right = normalize(cross(glm::vec3(0, 1, 0), forward));
    auto up = glm::normalize(cross(forward, right)); // 右手从x绕到y
    auto shadowCameraToWorld = glm::mat4(
        right.x, right.y, right.z, 0, // 第一列
        up.z, up.y, up.z, 0,
        forward.x, forward.y, forward.z, 0,
        0, 0, 0, 1); // 留空，之后设置
    auto worldToShadowCamera = inverse(shadowCameraToWorld);
    // 希望以摄像机为中心，但是先把摄像机位置转到阴影空间，然后对齐每个纹素，避免阴影光栅化时闪烁
    auto cameraPositionVS = worldToShadowCamera * glm::vec4(camera->position, 1);
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
    renderContext.SetViewProjMatrix(worldToShadowCamera, projMatrix);

    Material::SetGlobalMat4Value("_MainLightShadowVP", projMatrix * worldToShadowCamera);

    auto renderTarget = RenderTarget::Get(nullptr, m_mainLightShadowRt);
    renderTarget->Clear(1.0f);
    renderTarget->Use();

    renderContext.replaceMaterial = m_drawShadowMat;
    RenderingUtils::RenderScene(renderContext);
    renderContext.replaceMaterial = nullptr;

    // 准备绘制参数
    renderContext.SetViewProjMatrix(camera);
}

void MainLightShadowPass::UpdateRt(const RenderContext& renderContext)
{
    if (m_mainLightShadowRt == nullptr)
    {
        m_mainLightShadowRt = new RenderTexture(
            RenderTextureDescriptor(
                renderContext.mainLightShadowSize,
                renderContext.mainLightShadowSize,
                Depth,
                Point,
                Clamp,
                "_MainLightShadowMapTex"));
        m_mainLightShadowRt->IncRef();
        Material::SetGlobalTextureValue("_MainLightShadowMapTex", m_mainLightShadowRt);
    }
    m_mainLightShadowRt->Resize(renderContext.mainLightShadowSize, renderContext.mainLightShadowSize);
}

