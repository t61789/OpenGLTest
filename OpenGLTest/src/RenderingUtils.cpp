#include "RenderingUtils.h"

#include <stack>

#include "RenderPipeline.h"

void RenderingUtils::RenderScene(const RenderContext& renderContext)
{
    // DFS地绘制场景
    std::stack<Object*> drawingStack;
    drawingStack.push(renderContext.scene->sceneRoot);
    while(!drawingStack.empty())
    {
        auto obj = drawingStack.top();
        drawingStack.pop();
        auto entity = dynamic_cast<Entity*>(obj);
        if(entity != nullptr)
        {
            Utils::BeginDebugGroup("Draw Entity");
            RenderEntity(renderContext, entity);
            Utils::EndDebugGroup();
        }
        if(obj != nullptr)
        {
            for (auto& child : obj->children)
            {
                drawingStack.push(child);
            }
        }
    }
}

void RenderingUtils::RenderEntity(const RenderContext& renderContext, const Entity* entity)
{
    if(entity == nullptr)
    {
        return;
    }

    Mesh* mesh = entity->mesh;
    Material* material;
    if(renderContext.replaceMaterial != nullptr)
    {
        material = renderContext.replaceMaterial;
    }
    else
    {
        material = entity->material;
    }
    
    if(mesh == nullptr || material == nullptr)
    {
        return;
    }

    RenderMesh(renderContext, mesh, material, entity->GetLocalToWorld());
}

void RenderingUtils::RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const glm::mat4& m)
{
    auto mvp = renderContext.vpMatrix * m;

    mesh->use();
    mat->SetMat4Value("_MVP", mvp);
    mat->SetMat4Value("_ITM", transpose(inverse(m)));
    mat->SetMat4Value("_M", m);
    mat->Use(mesh);
    renderContext.cullModeMgr->SetCullMode(mat->cullMode);
    
    glDrawElements(GL_TRIANGLES, static_cast<GLint>(mesh->indicesCount), GL_UNSIGNED_INT, nullptr);
}
