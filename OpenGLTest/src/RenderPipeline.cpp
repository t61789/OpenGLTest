#include "RenderPipeline.h"

#include <queue>
#include <stack>
#include <ext/matrix_clip_space.hpp>

#include "Camera.h"
#include "Entity.h"
#include "GameFramework.h"

void RenderEntity(const Entity* entity, const RenderContext& renderContext);

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    m_window = window;
    SetScreenSize(width, height);
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::SetScreenSize(const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void RenderPipeline::Render(const RESOURCE_ID cameraId, const Scene* scene) const
{
    if(scene == nullptr)
    {
        return;
    }
    
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    auto sceneRoot = ResourceMgr::GetPtr<Object>(scene->m_sceneRoot);
    if(camera == nullptr || sceneRoot == nullptr)
    {
        return;
    }
    
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 准备绘制参数
    auto cameraLocalToWorld = camera->GetLocalToWorld();
    auto viewMatrix = inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(camera->fov),
        static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight),
        camera->nearClip,
        camera->farClip);
    auto vpMatrix = projectionMatrix * viewMatrix;

    RenderContext renderContext;
    renderContext.m_vpMatrix = vpMatrix;
    renderContext.m_cameraPositionWS = camera->m_position;
    renderContext.m_lightDirection = scene->m_lightDirection;

    // DFS地绘制场景
    std::stack<RESOURCE_ID> drawingStack;
    drawingStack.push(scene->m_sceneRoot);
    while(!drawingStack.empty())
    {
        auto entityId = drawingStack.top();
        drawingStack.pop();
        auto entity = ResourceMgr::GetPtr<Entity>(entityId);
        if(entity != nullptr)
        {
            RenderEntity(entity, renderContext);
        }
        auto object = ResourceMgr::GetPtr<Object>(entityId);
        if(object != nullptr)
        {
            for (auto& child : object->m_children)
            {
                drawingStack.push(child);
            }
        }
    }

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void RenderEntity(const Entity* entity, const RenderContext& renderContext)
{
    if(entity == nullptr)
    {
        return;
    }
    auto mesh = ResourceMgr::GetPtr<Mesh>(entity->m_mesh);
    auto material = ResourceMgr::GetPtr<Material>(entity->m_material);
    if(mesh == nullptr || material == nullptr)
    {
        return;
    }
    auto shader = ResourceMgr::GetPtr<Shader>(material->m_shader);
    if(shader == nullptr)
    {
        return;
    }

    auto m = entity->GetLocalToWorld();
    auto mvp = renderContext.m_vpMatrix * m;

    material->SetMat4Value("_MVP", mvp);
    material->SetMat4Value("_ITM", transpose(inverse(m)));
    material->SetMat4Value("_M", m);
    material->SetVector4Value("_CameraPositionWS", glm::vec4(renderContext.m_cameraPositionWS, 1));

    mesh->Use();
    shader->Use(mesh);
    material->FillParams(shader);
    
    glDrawElements(GL_TRIANGLES, mesh->m_indicesCount, GL_UNSIGNED_INT, 0);
}

