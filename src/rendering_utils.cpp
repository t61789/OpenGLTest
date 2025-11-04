#include "rendering_utils.h"

#include <tracy/Tracy.hpp>

#include "render_texture.h"
#include "mesh.h"

#include "utils.h"
#include "render_context.h"
#include "built_in_res.h"
#include "game_resource.h"
#include "material.h"
#include "object.h"
#include "shader.h"
#include "objects/render_comp.h"
#include "render/per_object_buffer.h"
#include "render/render_target_pool.h"
#include "render/texture_set.h"
#include "render/gl/gl_cbuffer.h"
#include "render/gl/gl_state.h"

namespace op
{
    using namespace std;

    void RenderingUtils::RenderScene(crvec<RenderComp*> renderComps)
    {
        for (auto& renderObj : renderComps)
        {
            RenderEntity(renderObj);
        }
    }

    void RenderingUtils::RenderEntity(const RenderComp* renderComp)
    {
        if(renderComp == nullptr)
        {
            return;
        }

        auto mesh = renderComp->GetMesh().get();
        auto material = renderComp->GetMaterial().get();
        if(mesh == nullptr || material == nullptr)
        {
            return;
        }

        RenderMesh({
            mesh,
            material,
            renderComp->HasOddNegativeScale(),
            renderComp->GetObjectIndex()
        });
    }

    void RenderingUtils::Blit(crsp<RenderTexture> src, crsp<RenderTexture> dst, Material* material)
    {
        auto usingRenderTarget = RenderTarget::Using(dst);
        
        auto blitMat = material ? material : GetBR()->blitMatNew.get();
        if (src)
        {
            blitMat->SetTexture(MAIN_TEX, src);
        }

        RenderMesh({
            GetBR()->quadMesh.get(),
            blitMat
        });
    }

    void RenderingUtils::RenderMesh(cr<RenderParam> renderParam)
    {
        ZoneScoped;
        
        BindDrawResources(renderParam);
        CallGlDraw(renderParam.mesh);
    }

    void RenderingUtils::BindDrawResources(cr<RenderParam> renderParam)
    {
        auto shader = renderParam.material->GetShader();
        auto mesh = renderParam.mesh;
        auto material = renderParam.material;
        
        // Bind Shader
        shader->Use();

        // Bind Predefined CBuffers
        GetGlobalCbuffer()->BindBase();
        GetPerViewCbuffer()->BindBase();
        if (renderParam.objectIndex.has_value())
        {
            GetGR()->GetPredefinedCbuffer(OBJECT_INDEX_CBUFFER)->Set(OBJECT_INDEX, renderParam.objectIndex.value());
            GetGR()->GetPredefinedCbuffer(OBJECT_INDEX_CBUFFER)->BindBase();
            GetGR()->GetPerObjectBuffer()->Use();
        }

        // Bind Vertex Attrib
        if (mesh)
        {
            mesh->Use();
        }
        
        // Bind Textures
        material->GetTextureSet()->ApplyTextures(shader.get());
        
        // Bind CBuffer
        material->UseCBuffer();
        
        // Set Render State
        GlState::Ins()->SetCullMode(material->cullMode, renderParam.hasOddNegativeScale);
        GlState::Ins()->SetBlendMode(material->blendMode);
        GlState::Ins()->SetDepthMode(material->depthMode, material->depthWrite);
    }
    
    void RenderingUtils::CallGlDraw(const Mesh* mesh)
    {
        GlState::GlDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, nullptr);
    }
}
