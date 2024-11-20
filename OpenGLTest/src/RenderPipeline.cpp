#include "RenderPipeline.h"

#include <iostream>

#include "GameFramework.h"

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

void RenderPipeline::Render(const Camera* camera) const
{
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto cameraLocalToWorld = camera->GetLocalToWorld();
    auto viewMatrix = inverse(cameraLocalToWorld);
    auto projectionMatrix = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(m_ScreenWidth) / static_cast<float>(m_ScreenHeight),
        0.1f,
        30.0f);
    auto vpMatrix = projectionMatrix * viewMatrix;
    
    for (auto& entity : m_Entities)
    {
        RenderEntity(entity, vpMatrix, camera->m_position);
    }

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

bool RenderPipeline::AddEntity(Entity* entity)
{
    for (auto& curEntity : m_Entities)
    {
        if(curEntity == entity)
        {
            return false;
        }
    }

    m_Entities.push_back(entity);
    return true;
}

bool RenderPipeline::RemoveEntity(const Entity* entity)
{
    for (size_t i = 0; i < m_Entities.size(); ++i)
    {
        if(m_Entities[i] == entity)
        {
            m_Entities.erase(m_Entities.begin() + i);
            return true;
        }
    }

    return false;
}

void RenderPipeline::RenderEntity(const Entity* entity, const glm::mat4& vpMatrix, const glm::vec3& cameraPositionWS)
{
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
