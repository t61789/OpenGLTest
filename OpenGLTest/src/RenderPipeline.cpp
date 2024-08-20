#include "RenderPipeline.h"

#include <iostream>

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
        RenderEntity(entity, vpMatrix);
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

void RenderPipeline::RenderEntity(const Entity* entity, const glm::mat4& vpMatrix)
{
    if(entity->mesh == nullptr || entity->shader == nullptr)
    {
        return;
    }

    auto mvp = vpMatrix * entity->GetLocalToWorld();

    entity->shader->SetMatrix("_MVP", mvp);

    entity->shader->Use();
    
    entity->mat->FillParams(entity->shader);

    entity->mesh->Use();

    glDrawElements(GL_TRIANGLES, entity->mesh->vertexCount, GL_UNSIGNED_INT, 0);
}
