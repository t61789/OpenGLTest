#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <ext/matrix_clip_space.hpp>
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

    m_fullScreenQuad = Mesh::LoadFromFile("Meshes/fullScreenMesh.obj");
    m_deferredShadingShader = Shader::LoadFromFile("Shaders/DeferredShading.glsl");
    m_finalBlitShader = Shader::LoadFromFile("Shaders/FinalBlit.glsl");

    std::vector<RenderTargetAttachment> attachments;
    attachments.emplace_back(GL_COLOR_ATTACHMENT0, glm::vec4(0.5),RenderTextureDescriptor(0,0,RGBAHdr, Point, Clamp));
    attachments.emplace_back(GL_COLOR_ATTACHMENT1, glm::vec4(0), RenderTextureDescriptor(0,0,RGBA, Point, Clamp));
    attachments.emplace_back(GL_COLOR_ATTACHMENT2, glm::vec4(0), RenderTextureDescriptor(0,0,RGBA, Point, Clamp));
    attachments.emplace_back(GL_DEPTH_ATTACHMENT, glm::vec4(0), RenderTextureDescriptor(0,0,Depth, Point, Clamp));
    m_gBufferRenderTarget = (new RenderTarget(attachments, 3))->id;

    attachments.clear();
    attachments.emplace_back(GL_COLOR_ATTACHMENT0, glm::vec4(0.77f, 0.77f, 0.83f, 1), RenderTextureDescriptor(0,0,RGBAHdr, Point, Clamp));
    m_shadingRenderTarget = (new RenderTarget(attachments, 1))->id;
}

RenderPipeline::~RenderPipeline()
{
    ResourceMgr::DeleteResource(m_fullScreenQuad);
    ResourceMgr::DeleteResource(m_deferredShadingShader);
    ResourceMgr::DeleteResource(m_gBufferRenderTarget);
    ResourceMgr::DeleteResource(m_shadingRenderTarget);
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

    _clearRenderTargetsPass();
    _renderScenePass(cameraId, scene, renderContext);
    _deferredShadingPass();
    _finalBlitPass();

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool RenderPipeline::_updateRenderTargetsPass()
{
    auto shadingRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_shadingRenderTarget);
    if(shadingRenderTarget != nullptr && shadingRenderTarget->width == m_screenWidth && shadingRenderTarget->height == m_screenHeight)
    {
        return true;
    }

    if(!shadingRenderTarget->createAttachmentsRt(m_screenWidth, m_screenHeight))
    {
        return false;
    }
    
    Material::SetGlobalRenderTextureValue("_ShadingBufferRt", shadingRenderTarget->getRenderTexture(0));

    auto gBufferRenderTarget = ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget);
    if(!gBufferRenderTarget->createAttachmentsRt(m_screenWidth, m_screenHeight))
    {
        return false;
    }

    Material::SetGlobalRenderTextureValue("_GBuffer0Rt", gBufferRenderTarget->getRenderTexture(0));
    Material::SetGlobalRenderTextureValue("_GBuffer1Rt", gBufferRenderTarget->getRenderTexture(1));

    return true;
}

void RenderPipeline::_clearRenderTargetsPass()
{
    ResourceMgr::GetPtr<RenderTarget>(m_gBufferRenderTarget)->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ResourceMgr::GetPtr<RenderTarget>(m_shadingRenderTarget)->clear(GL_COLOR_BUFFER_BIT);

    RenderTarget::ClearFrameBuffer(0, glm::vec4(0), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    // 准备绘制参数
    auto cameraLocalToWorld = camera->getLocalToWorld();
    auto viewMatrix = inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(camera->fov * 0.5f),
        static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight),
        camera->nearClip,
        camera->farClip);
    auto vpMatrix = projectionMatrix * viewMatrix;

    Material::SetGlobalVector4Value("_CameraPositionWS", glm::vec4(camera->position, 0));
    Material::SetGlobalVector4Value("_MainLightDirection", glm::vec4(normalize(scene->mainLightDirection), 0));
    Material::SetGlobalVector4Value("_MainLightColor", glm::vec4(scene->mainLightColor, 0));
    Material::SetGlobalVector4Value("_AmbientLightColor", glm::vec4(scene->ambientLightColor, 0));
    Material::SetGlobalFloatValue("_ExposureMultiplier", scene->tonemappingExposureMultiplier);
    Material::SetGlobalMat4Value("_IVP", inverse(vpMatrix));
    
    renderContext.vpMatrix = vpMatrix;
    renderContext.cameraPositionWS = camera->position;
    renderContext.mainLightDirection = normalize(scene->mainLightDirection);
    renderContext.mainLightColor = scene->mainLightColor;
    renderContext.ambientLightColor = scene->ambientLightColor;
    renderContext.tonemappingExposureMultiplier = scene->tonemappingExposureMultiplier;

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
    auto material = ResourceMgr::GetPtr<Material>(entity->material);
    if(mesh == nullptr || material == nullptr)
    {
        return;
    }
    auto shader = ResourceMgr::GetPtr<Shader>(material->shaderId);
    if(shader == nullptr)
    {
        return;
    }

    auto m = entity->getLocalToWorld();
    auto mvp = renderContext.vpMatrix * m;

    mesh->use();
    shader->use(mesh);
    material->fillParams(shader);

    shader->setMatrix("_MVP", mvp);
    shader->setMatrix("_ITM", transpose(inverse(m)));
    shader->setMatrix("_M", m);
    
    glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
}

void RenderPipeline::_deferredShadingPass()
{
    ResourceMgr::GetPtr<RenderTarget>(m_shadingRenderTarget)->use();

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
