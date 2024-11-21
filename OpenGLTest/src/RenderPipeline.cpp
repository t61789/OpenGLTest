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
    setScreenSize(width, height);
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::setScreenSize(const int width, const int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void RenderPipeline::render(const RESOURCE_ID cameraId, const Scene* scene) const
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
        glm::radians(camera->fov),
        static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight),
        camera->nearClip,
        camera->farClip);
    auto vpMatrix = projectionMatrix * viewMatrix;

    RenderContext renderContext;
    renderContext.vpMatrix = vpMatrix;
    renderContext.cameraPositionWS = camera->position;
    renderContext.mainLightDirection = normalize(scene->mainLightDirection);

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
            RenderEntity(entity, renderContext);
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

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void RenderEntity(const Entity* entity, const RenderContext& renderContext)
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
    shader->setVector("_MainLightColor", glm::vec4(renderContext.mainLightDirection, 1));
    shader->setVector("_AmbientLightColor", glm::vec4(renderContext.ambientLightColor, 1));
    
    glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
}

