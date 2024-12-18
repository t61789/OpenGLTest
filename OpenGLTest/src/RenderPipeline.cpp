﻿#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <ext/matrix_clip_space.hpp>
#include <ext/matrix_transform.hpp>
#include <ext/quaternion_common.hpp>

#include "Camera.h"
#include "Entity.h"
#include "GameFramework.h"
#include "RenderTarget.h"
#include "RenderTexture.h"

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    m_window = window;
    setScreenSize(width, height);

    m_sphereMesh = Mesh::LoadFromFile("Meshes/sphere.obj");

    m_skyboxMat = Material::CreateEmptyMaterial("Shaders/SkyboxShader.glsl");
    m_drawShadowMat = Material::CreateEmptyMaterial("Shaders/DrawShadow.glsl");

    m_fullScreenQuad = Mesh::LoadFromFile("Meshes/fullScreenMesh.obj");
    m_deferredShadingShader = Shader::LoadFromFile("Shaders/DeferredShading.glsl");
    m_finalBlitShader = Shader::LoadFromFile("Shaders/FinalBlit.glsl");
    

    m_gBuffer0Tex = (new RenderTexture(RenderTextureDescriptor(width, height, RGBAHdr, Point, Clamp, "_GBuffer0Tex")))->id;
    m_gBuffer1Tex = (new RenderTexture(RenderTextureDescriptor(width, height, RGBA, Point, Clamp, "_GBuffer1Tex")))->id;
    m_gBuffer2Tex = (new RenderTexture(RenderTextureDescriptor(width, height, DepthTex, Point, Clamp, "_GBuffer2Tex")))->id;
    m_gBufferDepthTex = (new RenderTexture(RenderTextureDescriptor(width, height, Depth, Point, Clamp, "_GBufferDepthTex")))->id;
    m_shadingBufferTex = (new RenderTexture(RenderTextureDescriptor(width, height, RGBAHdr, Point, Clamp, "_ShadingBufferTex")))->id;
    m_mainLightShadowMapTex = (new RenderTexture(RenderTextureDescriptor(mainLightShadowTexSize, mainLightShadowTexSize, Depth, Point, Clamp, "_MainLightShadowMapTex")))->id;
    Material::SetGlobalTextureValue("_GBuffer0Tex", m_gBuffer0Tex);
    Material::SetGlobalTextureValue("_GBuffer1Tex", m_gBuffer1Tex);
    Material::SetGlobalTextureValue("_GBuffer2Tex", m_gBuffer2Tex);
    Material::SetGlobalTextureValue("_ShadingBufferTex", m_shadingBufferTex);
    Material::SetGlobalTextureValue("_MainLightShadowMapTex", m_mainLightShadowMapTex);
    

    std::vector<RenderTargetAttachment> attachments;
    attachments.emplace_back(GL_COLOR_ATTACHMENT0, glm::vec4(0.5), m_gBuffer0Tex);
    attachments.emplace_back(GL_COLOR_ATTACHMENT1, glm::vec4(0), m_gBuffer1Tex);
    attachments.emplace_back(GL_COLOR_ATTACHMENT2, glm::vec4(1), m_gBuffer2Tex);
    attachments.emplace_back(GL_DEPTH_ATTACHMENT, glm::vec4(0), m_gBufferDepthTex);
    auto gBufferRenderTarget = new RenderTarget(attachments, 3, "GBuffer");
    m_gBufferRenderTarget = gBufferRenderTarget->id;

    attachments.clear();
    attachments.emplace_back(GL_COLOR_ATTACHMENT0, glm::vec4(0.77f, 0.77f, 0.83f, 1), m_shadingBufferTex);
    auto shadingRenderTarget = new RenderTarget(attachments, 1, "ShadingBuffer");
    m_shadingRenderTarget = shadingRenderTarget->id;

    attachments.clear();
    attachments.emplace_back(GL_DEPTH_ATTACHMENT, glm::vec4(1), m_mainLightShadowMapTex);
    auto mainLightShadowRenderTarget = new RenderTarget(attachments, 1, "MainLightShadowMap");
    m_mainLightShadowRenderTarget = mainLightShadowRenderTarget->id;
}

RenderPipeline::~RenderPipeline()
{
    ResourceMgr::DeleteResource(m_sphereMesh);
    ResourceMgr::DeleteResource(m_skyboxMat);
    ResourceMgr::DeleteResource(m_drawShadowMat);
    ResourceMgr::DeleteResource(m_fullScreenQuad);
    ResourceMgr::DeleteResource(m_deferredShadingShader);
    ResourceMgr::DeleteResource(m_finalBlitShader);
    
    ResourceMgr::DeleteResource(m_gBuffer0Tex);
    ResourceMgr::DeleteResource(m_gBuffer1Tex);
    ResourceMgr::DeleteResource(m_gBuffer2Tex);
    ResourceMgr::DeleteResource(m_gBufferDepthTex);
    ResourceMgr::DeleteResource(m_shadingBufferTex);
    ResourceMgr::DeleteResource(m_mainLightShadowMapTex);
    
    ResourceMgr::DeleteResource(m_gBufferRenderTarget);
    ResourceMgr::DeleteResource(m_shadingRenderTarget);
    ResourceMgr::DeleteResource(m_mainLightShadowRenderTarget);
}

void RenderPipeline::setScreenSize(const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void RenderPipeline::render(const RESOURCE_ID cameraId, const Scene* scene)
{
    if(!_updateRenderTargetsPass())
    {
        throw std::runtime_error("RenderTarget创建失败");
    }

    RenderContext renderContext;

    _preparingPass(cameraId, renderContext);
    _renderMainLightShadowPass(cameraId, scene, renderContext);
    _renderSkyboxPass(cameraId, renderContext);
    _renderScenePass(cameraId, scene, renderContext);
    _deferredShadingPass();
    _finalBlitPass();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
    
    Utils::ClearGlError();
}

bool RenderPipeline::_updateRenderTargetsPass()
{
    auto gBuffer0Tex = ResourceMgr::GetPtr<RenderTexture>(m_gBuffer0Tex);
    if(gBuffer0Tex->width != m_screenWidth || gBuffer0Tex->height != m_screenHeight)
    {
        gBuffer0Tex->resize(m_screenWidth, m_screenHeight);
        ResourceMgr::GetPtr<RenderTexture>(m_gBuffer1Tex)->resize(m_screenWidth, m_screenHeight);
        ResourceMgr::GetPtr<RenderTexture>(m_gBuffer2Tex)->resize(m_screenWidth, m_screenHeight);
        ResourceMgr::GetPtr<RenderTexture>(m_gBufferDepthTex)->resize(m_screenWidth, m_screenHeight);
        ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget)->rebindAttachments();
        
        ResourceMgr::GetPtr<RenderTexture>(m_shadingBufferTex)->resize(m_screenWidth, m_screenHeight);
        ResourceMgr::GetPtr<RenderTarget>(m_shadingRenderTarget)->rebindAttachments();
    }

    auto mainLightShadowMapTex = ResourceMgr::GetPtr<RenderTexture>(m_mainLightShadowMapTex);
    if(mainLightShadowMapTex->width != mainLightShadowTexSize)
    {
        mainLightShadowMapTex->resize(mainLightShadowTexSize, mainLightShadowTexSize);
        ResourceMgr::GetPtr<RenderTarget>(m_mainLightShadowRenderTarget)->rebindAttachments();
    }

    return true;
}

void RenderPipeline::_preparingPass(const RESOURCE_ID cameraId, RenderContext& renderContext)
{
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    if(camera == nullptr)
    {
        return;
    }

    Material::SetGlobalVector4Value("_CameraPositionWS", glm::vec4(camera->position, 0));
    
    renderContext.cameraPositionWS = camera->position;
    
    auto gBufferRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget);
    gBufferRenderTarget->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderPipeline::_renderMainLightShadowPass(RESOURCE_ID cameraId, const Scene* scene, RenderContext& renderContext)
{
    if(scene == nullptr)
    {
        return;
    }

    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    if(camera == nullptr)
    {
        return;
    }

    constexpr float range = 10;
    float range2 = 50;
    auto distancePerTexel = range * 2 / static_cast<float>(mainLightShadowTexSize);
    // 计算阴影矩阵
    auto forward = normalize(scene->mainLightDirection);
    auto right = normalize(cross(glm::vec3(0, 1, 0), forward));
    auto up = normalize(cross(right, forward)); // 右手从x绕到y
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
    _setViewProjMatrix(worldToShadowCamera, projMatrix, renderContext);

    Material::SetGlobalMat4Value("_MainLightShadowVP", projMatrix * worldToShadowCamera);

    auto mainLightShadowRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_mainLightShadowRenderTarget);
    mainLightShadowRenderTarget->clear(GL_DEPTH_BUFFER_BIT);
    mainLightShadowRenderTarget->use();

    renderContext.replaceMaterial = m_drawShadowMat;
    _renderScene(scene, renderContext);
    renderContext.replaceMaterial = UNDEFINED_RESOURCE;

    // 准备绘制参数
    _setViewProjMatrix(cameraId, renderContext);
}

void RenderPipeline::_renderSkyboxPass(const RESOURCE_ID cameraId, const RenderContext& renderContext)
{
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    auto sphereMesh = ResourceMgr::GetPtr<Mesh>(m_sphereMesh);
    auto skyboxMat = ResourceMgr::GetPtr<Material>(m_skyboxMat);
    if(camera == nullptr || sphereMesh == nullptr || skyboxMat == nullptr)
    {
        return;
    }

    ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget)->use();
    
    auto m = glm::mat4(1);
    m = translate(m, camera->position);
    m = scale(m, glm::vec3(1));

    glDisable(GL_CULL_FACE);
    
    _renderMesh(sphereMesh, skyboxMat, m, renderContext);

    // glEnable(GL_CULL_FACE);
}

void RenderPipeline::_renderScenePass(RESOURCE_ID cameraId, const Scene* scene, RenderContext& renderContext)
{
    if(scene == nullptr)
    {
        return;
    }
    
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    auto sceneRootObj = ResourceMgr::GetPtr<Object>(scene->sceneRoot);
    if(camera == nullptr || sceneRootObj == nullptr)
    {
        return;
    }

    ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget)->use();

    Material::SetGlobalVector4Value("_MainLightDirection", glm::vec4(normalize(scene->mainLightDirection), 0));
    Material::SetGlobalVector4Value("_MainLightColor", glm::vec4(scene->mainLightColor, 0));
    Material::SetGlobalVector4Value("_AmbientLightColor", glm::vec4(scene->ambientLightColor, 0));
    Material::SetGlobalFloatValue("_ExposureMultiplier", scene->tonemappingExposureMultiplier);
    
    renderContext.mainLightDirection = normalize(scene->mainLightDirection);
    renderContext.mainLightColor = scene->mainLightColor;
    renderContext.ambientLightColor = scene->ambientLightColor;
    renderContext.tonemappingExposureMultiplier = scene->tonemappingExposureMultiplier;
    
    _renderScene(scene, renderContext);
}

void RenderPipeline::_deferredShadingPass()
{
    auto shadingRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_shadingRenderTarget);
    shadingRenderTarget->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadingRenderTarget->use();

    auto fullScreenQuad = ResourceMgr::GetPtr<Mesh>(m_fullScreenQuad);
    auto deferredShadingShader = ResourceMgr::GetPtr<Shader>(m_deferredShadingShader);
    
    if(fullScreenQuad == nullptr || deferredShadingShader == nullptr)
    {
        return;
    }

    fullScreenQuad->use();
    deferredShadingShader->use(fullScreenQuad);

    Material::FillGlobalParams(deferredShadingShader);

    glDrawElements(GL_TRIANGLES, fullScreenQuad->indicesCount, GL_UNSIGNED_INT, 0);
}

void RenderPipeline::_finalBlitPass()
{
    RenderTarget::ClearFrameBuffer(0, glm::vec4(0), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderTarget::UseScreenTarget();
    
    auto fullScreenQuad = ResourceMgr::GetPtr<Mesh>(m_fullScreenQuad);
    auto finalBlitShader = ResourceMgr::GetPtr<Shader>(m_finalBlitShader);

    if(fullScreenQuad == nullptr || finalBlitShader == nullptr)
    {
        return;
    }

    fullScreenQuad->use();
    finalBlitShader->use(fullScreenQuad);

    Material::FillGlobalParams(finalBlitShader);

    glDrawElements(GL_TRIANGLES, fullScreenQuad->indicesCount, GL_UNSIGNED_INT, 0);
}

void RenderPipeline::_renderScene(const Scene* scene, const RenderContext& renderContext)
{
    // DFS地绘制场景
    std::stack<RESOURCE_ID> drawingStack;
    drawingStack.push(scene->sceneRoot);
    while(!drawingStack.empty())
    {
        auto entityId = drawingStack.top();
        drawingStack.pop();
        auto entity = ResourceMgr::GetPtr<Entity>(entityId);
        if(entity != nullptr)
        {
            _renderEntity(entity, renderContext);
        }
        auto object = ResourceMgr::GetPtr<Object>(entityId);
        if(object != nullptr)
        {
            for (auto& child : object->children)
            {
                drawingStack.push(child);
            }
        }
    }
}

void RenderPipeline::_renderEntity(const Entity* entity, const RenderContext& renderContext)
{
    if(entity == nullptr)
    {
        return;
    }
    auto mesh = ResourceMgr::GetPtr<Mesh>(entity->mesh);
    
    Material* material;
    if(renderContext.replaceMaterial != UNDEFINED_RESOURCE)
    {
        material = ResourceMgr::GetPtr<Material>(renderContext.replaceMaterial);
    }
    else
    {
        material = ResourceMgr::GetPtr<Material>(entity->material);
    }
    
    if(mesh == nullptr || material == nullptr)
    {
        return;
    }

    _renderMesh(mesh, material, entity->getLocalToWorld(), renderContext);
}

void RenderPipeline::_renderMesh(const Mesh* mesh, const Material* mat, const glm::mat4& m, const RenderContext& renderContext)
{
    auto shader = ResourceMgr::GetPtr<Shader>(mat->shaderId);
    if(shader == nullptr)
    {
        return;
    }
    
    auto mvp = renderContext.vpMatrix * m;

    mesh->use();
    shader->use(mesh);
    mat->fillParams(shader);

    shader->setMatrix("_MVP", mvp);
    shader->setMatrix("_ITM", transpose(inverse(m)));
    shader->setMatrix("_M", m);
    
    glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
}

void RenderPipeline::_setViewProjMatrix(RESOURCE_ID camera, RenderContext& renderContext)
{
    auto cameraPtr = ResourceMgr::GetPtr<Camera>(camera);
    if(cameraPtr == nullptr)
    {
        return;
    }

    auto cameraLocalToWorld = cameraPtr->getLocalToWorld();
    auto viewMatrix = inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(cameraPtr->fov * 0.5f),
        static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight),
        cameraPtr->nearClip,
        cameraPtr->farClip);
    _setViewProjMatrix(viewMatrix, projectionMatrix, renderContext);
}

void RenderPipeline::_setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj, RenderContext& renderContext)
{
    auto vpMatrix = proj * view;
    renderContext.vpMatrix = vpMatrix;
    Material::SetGlobalMat4Value("_VP", vpMatrix);
    Material::SetGlobalMat4Value("_IVP", inverse(vpMatrix));
}
