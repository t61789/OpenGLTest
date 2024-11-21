#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <ext/matrix_clip_space.hpp>

#include "Camera.h"
#include "Entity.h"
#include "GameFramework.h"

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    m_window = window;
    setScreenSize(width, height);
    glGenFramebuffers(1, &this->m_frameBuffer);

    m_fullScreenQuad = Mesh::LoadFromFile("Meshes/fullScreenMesh.obj");

    m_blitShader = Shader::LoadFromFile("Shaders/Blit.vert", "Shaders/Blit.frag");
}

RenderPipeline::~RenderPipeline()
{
    glDeleteFramebuffers(1, &this->m_frameBuffer);

    ResourceMgr::DeleteResource(m_fullScreenQuad);
    ResourceMgr::DeleteResource(m_blitShader);
}

void RenderPipeline::setScreenSize(const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void RenderPipeline::render(const RESOURCE_ID cameraId, const Scene* scene)
{
    if(scene == nullptr)
    {
        return;
    }
    
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    auto sceneRoot = ResourceMgr::GetPtr<Object>(scene->sceneRoot);
    if(camera == nullptr || sceneRoot == nullptr)
    {
        return;
    }
    
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 准备绘制参数
    auto cameraLocalToWorld = camera->getLocalToWorld();
    auto viewMatrix = inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(camera->fov * 0.5f),
        static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight),
        camera->nearClip,
        camera->farClip);
    auto vpMatrix = projectionMatrix * viewMatrix;

    RenderContext renderContext;
    renderContext.vpMatrix = vpMatrix;
    renderContext.cameraPositionWS = camera->position;
    renderContext.mainLightDirection = normalize(scene->mainLightDirection);
    renderContext.mainLightColor = scene->mainLightColor;
    renderContext.ambientLightColor = scene->ambientLightColor;

    auto shader = ResourceMgr::GetPtr<Shader>(m_blitShader);
    auto fullScreenQuad = ResourceMgr::GetPtr<Mesh>(m_fullScreenQuad);

    if(shader == nullptr || fullScreenQuad == nullptr)
    {
        return;
    }

    fullScreenQuad->use();
    shader->use(fullScreenQuad);
    
    glDrawElements(GL_TRIANGLES, fullScreenQuad->indicesCount, GL_UNSIGNED_INT, 0);

    // DFS地绘制场景
    // std::stack<RESOURCE_ID> drawingStack;
    // drawingStack.push(scene->sceneRoot);
    // while(!drawingStack.empty())
    // {
    //     auto entityId = drawingStack.top();
    //     drawingStack.pop();
    //     auto entity = ResourceMgr::GetPtr<Entity>(entityId);
    //     if(entity != nullptr)
    //     {
    //         _renderEntity(entity, renderContext);
    //     }
    //     auto object = ResourceMgr::GetPtr<Object>(entityId);
    //     if(object != nullptr)
    //     {
    //         for (auto& child : object->children)
    //         {
    //             drawingStack.push(child);
    //         }
    //     }
    // }

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void RenderPipeline::_updateCameraAttachments()
{
    if(this->m_cameraColorAttachment != (GLuint)-1)
    {
        glDeleteTextures(1, &this->m_cameraColorAttachment);
        glDeleteTextures(1, &this->m_cameraDepthAttachment);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, this->m_frameBuffer);
    
    glGenTextures(1, &this->m_cameraColorAttachment);
    glGenTextures(1, &this->m_cameraDepthAttachment);

    glBindTexture(GL_TEXTURE_2D, this->m_cameraColorAttachment);
    glTexImage2D(
        this->m_cameraColorAttachment,
        0,
        GL_RGB,
        m_screenWidth,
        m_screenHeight,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->m_cameraColorAttachment, 0);
    
    glBindTexture(GL_TEXTURE_2D, this->m_cameraDepthAttachment);
    glTexImage2D(
        this->m_cameraDepthAttachment,
        0,
        GL_DEPTH24_STENCIL8,
        m_screenWidth,
        m_screenHeight,
        0,
        GL_DEPTH24_STENCIL8,
        GL_UNSIGNED_INT_24_8,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->m_cameraDepthAttachment, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Utils::LogError("FrameBufferAttachment绑定失败");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

