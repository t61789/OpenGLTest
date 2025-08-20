#include "rendering_utils.h"

#include <tracy/Tracy.hpp>

#include "render_target.h"
#include "render_texture.h"
#include "mesh.h"

#include "utils.h"
#include "render_context.h"
#include "built_in_res.h"
#include "game_resource.h"
#include "object.h"
#include "shader.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    using namespace std;

    void RenderingUtils::RenderScene(const vector<RenderComp*>& renderComps)
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

        auto rc = RenderContext::Ins();
        Mesh* mesh = renderComp->mesh;
        
        if(mesh == nullptr)
        {
            return;
        }

        RenderMesh(mesh, renderComp->material, renderComp->owner->transform->GetLocalToWorld(), renderComp->owner->transform->GetWorldToLocal());
    }

    void RenderingUtils::RenderMesh(Mesh* mesh, Material* mat, const Matrix4x4& m, const Matrix4x4& im)
    {
        ZoneScoped;
        
        auto perObjectMaterial = GameResource::Ins()->GetPredefinedMaterial(PER_OBJECT_CBUFFER.Hash());
        perObjectMaterial->Set(M, m);
        perObjectMaterial->Set(IM, im);
        
        BindDrawResources(mat->GetShader(), mesh, mat);
        CallGlDraw(mesh);
    }

    void RenderingUtils::Blit(RenderTexture* src, RenderTexture* dst, Material* material)
    {
        Material* blitMat = material ? material : BuiltInRes::Ins()->blitMatNew;
        Mesh* quad = BuiltInRes::Ins()->quadMesh;
        
        RenderTarget::Get(dst, nullptr)->Use();
        if (src) { blitMat->Set(MAIN_TEX, src); }
        
        BindDrawResources(blitMat->GetShader(), quad, blitMat);
        CallGlDraw(quad);
    }

    void RenderingUtils::ApplyTextures(Material* material, Shader* shader)
    {
        if (shader->textures.empty())
        {
            return;
        }
        
        static std::vector<Texture*> needBindingTextures;
        static std::vector<size_t> needBindingTexturesNameId;
        needBindingTextures.clear();
        needBindingTexturesNameId.clear();
        needBindingTextures.reserve(shader->textures.size());
        needBindingTexturesNameId.reserve(shader->textures.size());
        for (auto& [nameId, textureInfo] : shader->textures)
        {
            if (auto texture = material->GetTexture(nameId))
            {
                needBindingTextures.push_back(texture);
            }
            else
            {
                needBindingTextures.push_back(BuiltInRes::Ins()->missTex);
            }
            needBindingTexturesNameId.push_back(nameId);
        }

        auto rc = RenderContext::Ins();
        const auto& bindingInfos = rc->textureBindingMgr->BindTextures(needBindingTextures);
        for (size_t i = 0; i < bindingInfos.size(); i++)
        {
            shader->SetVal(needBindingTexturesNameId[i], bindingInfos[i].slot);
        }
        GL_CHECK_ERROR(ApplyTexture)
    }

    void RenderingUtils::CallGlDraw(const Mesh* mesh)
    {
        ZoneScoped;
        
        glDrawElements(GL_TRIANGLES, static_cast<GLint>(mesh->indicesCount), GL_UNSIGNED_INT, nullptr);
    }

    void RenderingUtils::BindDrawResources(Shader* shader, Mesh* mesh, Material* material)
    {
        // Bind Shader
        auto rs = RenderState::Ins();
        rs->SetShader(shader->glShaderId);
        
        auto gMat = GameResource::Ins()->GetPredefinedMaterial(GLOBAL_CBUFFER);
        auto vMat = GameResource::Ins()->GetPredefinedMaterial(PER_VIEW_CBUFFER);
        auto oMat = GameResource::Ins()->GetPredefinedMaterial(PER_OBJECT_CBUFFER);
        // if (rs->SetShader(shader->glShaderId))
        // {
            gMat->UseCBuffer();
            vMat->UseCBuffer();
            oMat->UseCBuffer();
        // }
        GL_CHECK_ERROR(BindPredefinedCBuffers)


        // Bind Vertex Attrib
        rs->SetVertexArray(mesh->vao);
        rs->BindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

        
        // Bind Textures
        ApplyTextures(material, shader);

        
        // Bind CBuffer
        material->UseCBuffer();

        
        // Set Render State
        rs->SetCullMode(material->cullMode);
        rs->SetBlendMode(material->blendMode);
    }
}
