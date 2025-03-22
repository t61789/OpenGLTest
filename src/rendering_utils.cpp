#include "RenderingUtils.h"

#include "RenderTarget.h"
#include "RenderTexture.h"
#include "Mesh.h"
#include "Material.h"
#include "Utils.h"
#include "RenderContext.h"
#include "BuiltInRes.h"
#include "Object.h"
#include "Objects/RenderComp.h"
#include "Objects/TransformComp.h"

namespace op
{
    using namespace std;

    void RenderingUtils::RenderScene(const RenderContext& renderContext, const vector<RenderComp*>& renderComps)
    {
        // DFS地绘制场景
        for (auto& renderObj : renderComps)
        {
            Utils::BeginDebugGroup(string("Draw Entity ") + renderObj->owner->name);
            RenderEntity(renderContext, renderObj);
            Utils::EndDebugGroup();
        }
    }

    void RenderingUtils::RenderEntity(const RenderContext& renderContext, const RenderComp* renderComp)
    {
        if(renderComp == nullptr)
        {
            return;
        }

        Mesh* mesh = renderComp->mesh;
        Material* material;
        if(renderContext.replaceMaterial != nullptr)
        {
            material = renderContext.replaceMaterial;
        }
        else
        {
            material = renderComp->material;
        }
        
        if(mesh == nullptr || material == nullptr)
        {
            return;
        }

        RenderMesh(renderContext, mesh, material, renderComp->owner->transform->GetLocalToWorld());
    }

    void RenderingUtils::RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const glm::mat4& m)
    {
        auto mvp = renderContext.vpMatrix * m;

        mesh->Use();
        mat->SetMat4Value("_MVP", mvp);
        mat->SetMat4Value("_VP", renderContext.vpMatrix);
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
        glDrawElements(GL_TRIANGLES, quad->indicesCount, GL_UNSIGNED_INT, nullptr);
    }
}
