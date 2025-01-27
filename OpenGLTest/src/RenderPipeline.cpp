#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <ext/matrix_clip_space.hpp>
#include <ext/matrix_transform.hpp>
#include <ext/quaternion_common.hpp>

#include "Camera.h"
#include "Entity.h"
#include "GameFramework.h"
#include "Gui.h"
#include "Image.h"
#include "RenderTarget.h"
#include "RenderTexture.h"

RenderPipeline* RenderPipeline::instance = nullptr;

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    instance = this;

    m_cullModeMgr = new CullModeMgr();
    
    m_window = window;
    setScreenSize(width, height);

    auto desc = ImageDescriptor::GetDefault();
    desc.needFlipVertical = false;
    m_skyboxCubeTexture = Image::LoadCubeFromFile("textures/skybox", "jpg", desc);
    m_lutTexture = Image::LoadFromFile("textures/testLut.png", ImageDescriptor::GetDefault());
    Material::SetGlobalTextureValue("_SkyboxTex", m_skyboxCubeTexture);

    m_sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
    
    m_skyboxMat = Material::LoadFromFile("materials/skybox_mat.json");
    m_drawShadowMat = Material::CreateEmptyMaterial("shaders/draw_shadow.glsl");
    m_deferredShadingMat = Material::CreateEmptyMaterial("shaders/deferred_shading.glsl");
    m_finalBlitMat = Material::CreateEmptyMaterial("shaders/final_blit.glsl");

    m_quadMesh = Mesh::LoadFromFile("meshes/quad.obj");

    m_gBuffer0Tex = (new RenderTexture(RenderTextureDescriptor(width, height, RGBAHdr, Point, Clamp, "_GBuffer0Tex")))->id;
    m_gBuffer1Tex = (new RenderTexture(RenderTextureDescriptor(width, height, RGBA, Point, Clamp, "_GBuffer1Tex")))->id;
    m_gBuffer2Tex = (new RenderTexture(RenderTextureDescriptor(width, height, DepthTex, Point, Clamp, "_GBuffer2Tex")))->id;
    m_gBufferDepthTex = (new RenderTexture(RenderTextureDescriptor(width, height, DepthStencil, Point, Clamp, "_GBufferDepthTex")))->id;
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
    attachments.emplace_back(GL_DEPTH_STENCIL_ATTACHMENT, glm::vec4(0), m_gBufferDepthTex);
    auto gBufferRenderTarget = new RenderTarget(attachments, 3, "GBuffer");
    m_gBufferRenderTarget = gBufferRenderTarget->id;

    attachments.clear();
    attachments.emplace_back(GL_COLOR_ATTACHMENT0, glm::vec4(0.77f, 0.77f, 0.83f, 1), m_shadingBufferTex);
    attachments.emplace_back(GL_DEPTH_STENCIL_ATTACHMENT, glm::vec4(0), m_gBufferDepthTex);
    auto shadingRenderTarget = new RenderTarget(attachments, 1, "ShadingBuffer");
    m_shadingRenderTarget = shadingRenderTarget->id;

    attachments.clear();
    attachments.emplace_back(GL_DEPTH_ATTACHMENT, glm::vec4(1), m_mainLightShadowMapTex);
    auto mainLightShadowRenderTarget = new RenderTarget(attachments, 1, "MainLightShadowMap");
    m_mainLightShadowRenderTarget = mainLightShadowRenderTarget->id;
}

RenderPipeline::~RenderPipeline()
{
    instance = nullptr;

    delete m_cullModeMgr;
    
    ResourceMgr::DeleteResource(m_skyboxCubeTexture);
    ResourceMgr::DeleteResource(m_lutTexture);
    
    ResourceMgr::DeleteResource(m_sphereMesh);
    ResourceMgr::DeleteResource(m_skyboxMat);
    ResourceMgr::DeleteResource(m_drawShadowMat);
    ResourceMgr::DeleteResource(m_deferredShadingMat);
    ResourceMgr::DeleteResource(m_finalBlitMat);
    ResourceMgr::DeleteResource(m_quadMesh);
    
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

    Utils::BeginDebugGroup("Preparing");
    _preparingPass(cameraId);
    Utils::EndDebugGroup();
    
    Utils::BeginDebugGroup("Draw Main Light Shadow");
    _renderMainLightShadowPass(cameraId, scene);
    Utils::EndDebugGroup();
    
    Utils::BeginDebugGroup("Draw Skybox");
    _renderSkyboxPass(cameraId);
    Utils::EndDebugGroup();
    
    Utils::BeginDebugGroup("Draw Scene");
    _renderScenePass(cameraId, scene);
    Utils::EndDebugGroup();
    
    Utils::BeginDebugGroup("Deferred Lighting");
    _deferredShadingPass();
    Utils::EndDebugGroup();
    
    Utils::BeginDebugGroup("Final Blit");
    _finalBlitPass();
    Utils::EndDebugGroup();
    
    Utils::BeginDebugGroup("Draw UI");
    _renderUiPass();
    Utils::EndDebugGroup();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
    
    Utils::ClearGlError();
}

void RenderPipeline::getViewProjMatrix(glm::mat4& view, glm::mat4& proj)
{
    view = m_renderContext.vMatrix;
    proj = m_renderContext.pMatrix;
}

void RenderPipeline::getScreenSize(int& width, int& height)
{
    width = this->m_screenWidth;
    height = this->m_screenHeight;
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

void RenderPipeline::_preparingPass(const RESOURCE_ID cameraId)
{
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    if(camera == nullptr)
    {
        return;
    }

    Material::SetGlobalVector4Value("_CameraPositionWS", glm::vec4(camera->position, 0));
    
    m_renderContext.cameraPositionWS = camera->position;
    
    auto gBufferRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget);
    gBufferRenderTarget->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderPipeline::_renderMainLightShadowPass(RESOURCE_ID cameraId, const Scene* scene)
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
    _setViewProjMatrix(worldToShadowCamera, projMatrix);

    Material::SetGlobalMat4Value("_MainLightShadowVP", projMatrix * worldToShadowCamera);

    auto mainLightShadowRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_mainLightShadowRenderTarget);
    mainLightShadowRenderTarget->clear(GL_DEPTH_BUFFER_BIT);
    mainLightShadowRenderTarget->use();

    m_renderContext.replaceMaterial = m_drawShadowMat;
    _renderScene(scene);
    m_renderContext.replaceMaterial = UNDEFINED_RESOURCE;

    // 准备绘制参数
    _setViewProjMatrix(cameraId);
}

void RenderPipeline::_renderSkyboxPass(const RESOURCE_ID cameraId)
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

    _renderMesh(sphereMesh, skyboxMat, m);
}

void RenderPipeline::_renderScenePass(RESOURCE_ID cameraId, const Scene* scene)
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
    
    m_renderContext.mainLightDirection = normalize(scene->mainLightDirection);
    m_renderContext.mainLightColor = scene->mainLightColor;
    m_renderContext.ambientLightColor = scene->ambientLightColor;
    m_renderContext.tonemappingExposureMultiplier = scene->tonemappingExposureMultiplier;
    
    _renderScene(scene);
}

void RenderPipeline::_deferredShadingPass()
{
    auto shadingRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_shadingRenderTarget);
    shadingRenderTarget->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadingRenderTarget->use();

    auto fullScreenQuad = ResourceMgr::GetPtr<Mesh>(m_quadMesh);
    auto deferredShadingMat = ResourceMgr::GetPtr<Material>(m_deferredShadingMat);
    
    if(fullScreenQuad == nullptr || deferredShadingMat == nullptr)
    {
        return;
    }

    _renderMesh(fullScreenQuad, deferredShadingMat, glm::mat4(1));
}

void RenderPipeline::_finalBlitPass()
{
    RenderTarget::ClearFrameBuffer(0, glm::vec4(0), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderTarget::UseScreenTarget();
    
    auto fullScreenQuad = ResourceMgr::GetPtr<Mesh>(m_quadMesh);
    auto finalBlitMat = ResourceMgr::GetPtr<Material>(m_finalBlitMat);

    if(fullScreenQuad == nullptr || finalBlitMat == nullptr)
    {
        return;
    }

    finalBlitMat->setTextureValue("_LutTex", m_lutTexture);
    _renderMesh(fullScreenQuad, finalBlitMat, glm::mat4()); 
}

void RenderPipeline::_renderUiPass()
{
    Gui::Render();
}

void RenderPipeline::_renderScene(const Scene* scene)
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
            Utils::BeginDebugGroup("Draw Entity");
            _renderEntity(entity);
            Utils::EndDebugGroup();
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

void RenderPipeline::_renderEntity(const Entity* entity)
{
    if(entity == nullptr)
    {
        return;
    }
    auto mesh = ResourceMgr::GetPtr<Mesh>(entity->mesh);
    
    Material* material;
    if(m_renderContext.replaceMaterial != UNDEFINED_RESOURCE)
    {
        material = ResourceMgr::GetPtr<Material>(m_renderContext.replaceMaterial);
    }
    else
    {
        material = ResourceMgr::GetPtr<Material>(entity->material);
    }
    
    if(mesh == nullptr || material == nullptr)
    {
        return;
    }

    _renderMesh(mesh, material, entity->getLocalToWorld());
}

void RenderPipeline::_renderMesh(const Mesh* mesh, Material* mat, const glm::mat4& m)
{
    auto mvp = m_renderContext.vpMatrix * m;

    mesh->use();
    mat->setMat4Value("_MVP", mvp);
    mat->setMat4Value("_ITM", transpose(inverse(m)));
    mat->setMat4Value("_M", m);
    mat->use(mesh);
    m_cullModeMgr->setCullMode(mat->cullMode);
    
    glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
}

void RenderPipeline::_setViewProjMatrix(RESOURCE_ID camera)
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
    _setViewProjMatrix(viewMatrix, projectionMatrix);
}

void RenderPipeline::_setViewProjMatrix(const glm::mat4& view, const glm::mat4& proj)
{
    m_renderContext.vMatrix = view;
    m_renderContext.pMatrix = proj;
    auto vpMatrix = proj * view;
    m_renderContext.vpMatrix = vpMatrix;
    Material::SetGlobalMat4Value("_VP", vpMatrix);
    Material::SetGlobalMat4Value("_IVP", inverse(vpMatrix));
}
