#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <ext/matrix_clip_space.hpp>

#include "Camera.h"
#include "Entity.h"
#include "GameFramework.h"
#include "RenderTexture.h"

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    m_window = window;
    setScreenSize(width, height);
    
    glGenFramebuffers(1, &this->m_backBuffer);
    
    m_fullScreenQuad = Mesh::LoadFromFile("Meshes/fullScreenMesh.obj");
    m_blitShader = Shader::LoadFromFile("Shaders/FinalBlit.vert", "Shaders/FinalBlit.frag");
}

RenderPipeline::~RenderPipeline()
{
    glDeleteFramebuffers(1, &this->m_backBuffer);

    ResourceMgr::DeleteResource(m_fullScreenQuad);
    ResourceMgr::DeleteResource(m_blitShader);
    ResourceMgr::DeleteResource(m_cameraColorAttachment);
    ResourceMgr::DeleteResource(m_cameraDepthAttachment);
}

void RenderPipeline::setScreenSize(const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void RenderPipeline::render(const RESOURCE_ID cameraId, const Scene* scene)
{
    if(!_updateCameraAttachments())
    {
        return;
    }

    RenderContext renderContext;

    _clearAttachments();
    
    _renderScene(cameraId, scene, renderContext);

    _blitAttachmentToScreen(renderContext);
    
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool RenderPipeline::_updateCameraAttachments()
{
    auto colorAttachment = ResourceMgr::GetPtr<RenderTexture>(m_cameraColorAttachment);
    
    if(colorAttachment != nullptr && colorAttachment->width == m_screenWidth && colorAttachment->height == m_screenHeight)
    {
        return true;
    }

    if(colorAttachment != nullptr)
    {
        ResourceMgr::DeleteResource(m_cameraColorAttachment);
        ResourceMgr::DeleteResource(m_cameraDepthAttachment);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, this->m_backBuffer);

    colorAttachment = new RenderTexture(m_screenWidth, m_screenHeight, RGBAHdr, Bilinear, Clamp);
    m_cameraColorAttachment = colorAttachment->id;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->glTextureId, 0);

    auto depthStencilAttachment = new RenderTexture(m_screenWidth, m_screenHeight, Depth, Point, Clamp);
    m_cameraDepthAttachment = depthStencilAttachment->id;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthStencilAttachment->glTextureId, 0);

    auto result = true;
    auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        Utils::LogError("FrameBufferAttachment绑定失败：" + std::to_string(frameBufferStatus));
        result = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return result;
}

void RenderPipeline::_clearAttachments()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, this->m_backBuffer);
    auto colorAttachment = ResourceMgr::GetPtr<RenderTexture>(m_cameraColorAttachment);
    glViewport(0, 0, colorAttachment->width, colorAttachment->height);
    glClearColor(0.77f, 0.77f, 0.83f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderPipeline::_renderScene(RESOURCE_ID cameraId, const Scene* scene, RenderContext& renderContext)
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
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->m_backBuffer);

    // 准备绘制参数
    auto cameraLocalToWorld = camera->getLocalToWorld();
    auto viewMatrix = inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(camera->fov * 0.5f),
        static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight),
        camera->nearClip,
        camera->farClip);
    auto vpMatrix = projectionMatrix * viewMatrix;
    
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
    shader->setVector("_CameraPositionWS", glm::vec4(renderContext.cameraPositionWS, 1));
    shader->setVector("_MainLightDirection", glm::vec4(renderContext.mainLightDirection, 1));
    shader->setVector("_MainLightColor", glm::vec4(renderContext.mainLightColor, 1));
    shader->setVector("_AmbientLightColor", glm::vec4(renderContext.ambientLightColor, 1));
    
    glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
}

void RenderPipeline::_blitAttachmentToScreen(const RenderContext& renderContext)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, m_screenWidth, m_screenHeight);
    
    Mesh* fullScreenQuad = ResourceMgr::GetPtr<Mesh>(m_fullScreenQuad);
    Shader* blitShader = ResourceMgr::GetPtr<Shader>(m_blitShader);
    
    if(fullScreenQuad == nullptr || blitShader == nullptr)
    {
        return;
    }
    
    fullScreenQuad->use();
    blitShader->use(fullScreenQuad);
    
    blitShader->setRenderTexture("_MainTex", 0, m_cameraColorAttachment);
    blitShader->setFloat("_ExposureMultiplier", renderContext.tonemappingExposureMultiplier);

    glDrawElements(GL_TRIANGLES, fullScreenQuad->indicesCount, GL_UNSIGNED_INT, 0);
}
