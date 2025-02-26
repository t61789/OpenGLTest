#include "RenderingUtils.h"

#include <stack>

#include "BuiltInRes.h"
#include "RenderPipeline.h"

using namespace std;

void RenderingUtils::RenderScene(const RenderContext& renderContext, const vector<Entity*>& renderObjs)
{
    // DFS地绘制场景
    for (auto& renderObj : renderObjs)
    {
        Utils::BeginDebugGroup(string("Draw Entity ") + renderObj->name);
        RenderEntity(renderContext, renderObj);
        Utils::EndDebugGroup();
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

    mesh->Use();
    mat->SetMat4Value("_MVP", mvp);
    mat->SetMat4Value("_ITM", transpose(inverse(m)));
    mat->SetMat4Value("_M", m);
    mat->Use(mesh);
    renderContext.cullModeMgr->SetCullMode(mat->cullMode);
    
    glDrawElements(GL_TRIANGLES, static_cast<GLint>(mesh->indicesCount), GL_UNSIGNED_INT, nullptr);
}

void RenderingUtils::Blit(RenderTexture* src, RenderTexture* dst, Material* material)
{
    auto quad = BuiltInRes::GetInstance()->quadMesh;
    
    Material* blitMat;
    if (material)
    {
        blitMat = material;
    }
    else
    {
        blitMat = BuiltInRes::GetInstance()->blitMat;
    }
    
    if (src)
    {
        blitMat->SetTextureValue("_MainTex", src);
    }

    RenderTarget::Get(dst, nullptr)->Use();
    quad->Use();
    blitMat->Use(quad);
    glDrawElements(GL_TRIANGLES, static_cast<GLint>(quad->indicesCount), GL_UNSIGNED_INT, nullptr);
}
