#include "RenderPipeline.h"

#include <iostream>
#include <queue>

#include "GameFramework.h"
#include "Utils.h"

void RenderEntity(const Entity* entity, const glm::mat4& vpMatrix, const glm::vec3& cameraPositionWS);

RenderPipeline::RenderPipeline(const int width, const int height, GLFWwindow* window)
{
    m_Window = window;
    SetScreenSize(width, height);
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::SetScreenSize(const int width, const int height)
{
    m_ScreenWidth = width;
    m_ScreenHeight = height;
}

void RenderPipeline::Render(const RESOURCE_ID cameraId, const RESOURCE_ID sceneRootId) const
{
    auto camera = ResourceMgr::GetPtr<Camera>(cameraId);
    auto sceneRoot = ResourceMgr::GetPtr<Entity>(sceneRootId);
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
        static_cast<float>(m_ScreenWidth) / static_cast<float>(m_ScreenHeight),
        camera->nearClip,
        camera->farClip);
    auto vpMatrix = projectionMatrix * viewMatrix;

    // BFS地绘制场景
    std::queue<RESOURCE_ID> entityQueue;
    entityQueue.push(sceneRootId);
    while(!entityQueue.empty())
    {
        auto entityId = entityQueue.front();
        entityQueue.pop();
        auto entity = ResourceMgr::GetPtr<Entity>(entityId);
        if(entity != nullptr)
        {
            RenderEntity(entity, vpMatrix, camera->m_position);
        }
        auto object = ResourceMgr::GetPtr<Object>(entityId);
        if(object != nullptr)
        {
            for (auto& child : object->m_children)
            {
                entityQueue.push(child);
            }
        }
    }

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

void RenderEntity(const Entity* entity, const glm::mat4& vpMatrix, const glm::vec3& cameraPositionWS)
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
    auto mvp = vpMatrix * m;

    material->SetMat4Value("_MVP", mvp);
    material->SetMat4Value("_ITM", transpose(inverse(m)));
    material->SetMat4Value("_M", m);
    material->SetVector4Value("_CameraPositionWS", glm::vec4(cameraPositionWS, 1));

    mesh->Use();
    shader->Use(mesh);
    material->FillParams(shader);
    
    glDrawElements(GL_TRIANGLES, mesh->m_vertexCount, GL_UNSIGNED_INT, 0);
}

