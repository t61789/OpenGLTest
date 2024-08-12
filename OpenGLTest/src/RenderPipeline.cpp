#include "RenderPipeline.h"

#include "Material.h"

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

void RenderPipeline::Render() const
{
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    auto viewMatrix = glm::mat4(1.0f);
    // matrix[i] 取的是矩阵的第i列
    // 填写每一个单元时，先列再行
    viewMatrix[3][1] = -1.0f;
    viewMatrix[3][2] = -3.0f;
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

    auto objectMatrix = glm::mat4(1);
    objectMatrix = translate(objectMatrix, entity->rotation);
    auto mvp = vpMatrix * objectMatrix;

    entity->shader->setMatrix("_MVP", mvp);

    entity->shader->use();
    
    entity->mat->FillParams(entity->shader);

    entity->mesh->use();

    glDrawElements(GL_TRIANGLES, entity->mesh->vertexCount, GL_UNSIGNED_INT, 0);
}
