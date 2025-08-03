#include "rendering_utils.h"

#include <tracy/Tracy.hpp>

#include "render_target.h"
#include "render_texture.h"
#include "mesh.h"
#include "material.h"
#include "utils.h"
#include "render_context.h"
#include "built_in_res.h"
#include "cull_mode.h"
#include "blend_mode.h"
#include "object.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    using namespace std;

    void RenderingUtils::RenderScene(const RenderContext& renderContext, const vector<RenderComp*>& renderComps)
    {
        for (auto& renderObj : renderComps)
        {
            RenderEntity(renderContext, renderObj);
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

    void RenderingUtils::RenderMesh(const RenderContext& renderContext, const Mesh* mesh, Material* mat, const Matrix4x4& m)
    {
        ZoneScoped;
        
        // auto mm = Matrix4x4(
        //     0.01f, 0, 0, 0,
        //     0, 0.01f, 0, 0,
        //     0, 0, 0.01f, 0,
        //     0, 0, 0, 1);
        // auto mm = Matrix4x4(
        //     1, 0, 0, 0,
        //     0, 1, 0, 0,
        //     0, 0, 1, 0,
        //     0, 0, 0, 1);

        // auto vv = Matrix4x4(
        //     1, 0, 0, 0,
        //     0, 1, 0, 0,
        //     0, 0, -1, -10,
        //     0, 0, 0, 1).Inverse();
        //
        // Utils::Log(Info, "vv %s", vv.ToString().c_str());
        // Utils::Log(Info, "v %s", renderContext.vMatrix0.ToString().c_str());
        

        auto mvp = renderContext.vpMatrix * m;
        auto itm = m.Inverse().Transpose();
        
        mesh->Use();
        mat->SetMat4Value(MVP, mvp);
        mat->SetMat4Value(VP, renderContext.vpMatrix);
        mat->SetMat4Value(ITM, itm);
        mat->SetMat4Value(M, m);
        mat->Use(mesh);
        renderContext.cullModeMgr->SetCullMode(mat->cullMode);
        renderContext.blendModeMgr->SetBlendMode(mat->blendMode);

        CallGlDraw(mesh);
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
            blitMat->SetTextureValue(MAIN_TEX, src);
        }

        RenderTarget::Get(dst, nullptr)->Use();
        quad->Use();
        blitMat->Use(quad);
        glDrawElements(GL_TRIANGLES, quad->indicesCount, GL_UNSIGNED_INT, nullptr);
    }

    void RenderingUtils::CallGlDraw(const Mesh* mesh)
    {
        ZoneScoped;
        
        glDrawElements(GL_TRIANGLES, static_cast<GLint>(mesh->indicesCount), GL_UNSIGNED_INT, nullptr);
    }
}
