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

        Mesh* mesh = renderComp->mesh;
        if(mesh == nullptr)
        {
            return;
        }

        RenderMesh({
            mesh,
            renderComp->material,
            &renderComp->GetOwner()->transform->GetLocalToWorld(),
            &renderComp->GetOwner()->transform->GetWorldToLocal(),
            renderComp->GetObjectIndex()
        });
    }

    void RenderingUtils::RenderMesh(const RenderParam& renderParam)
    {
        ZoneScoped;
        
        BindDrawResources(renderParam);
        CallGlDraw(renderParam.mesh);
    }

    void RenderingUtils::Blit(RenderTexture* src, RenderTexture* dst, Material* material)
    {
        Material* blitMat = material ? material : BUILT_IN_RES->blitMatNew;
        Mesh* quad = BUILT_IN_RES->quadMesh;
        
        RenderTarget::Get(dst, nullptr)->Use();
        if (src) { blitMat->Set(MAIN_TEX, src); }

        RenderMesh({
            quad,
            blitMat
        });
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
                needBindingTextures.push_back(BUILT_IN_RES->missTex);
            }
            needBindingTexturesNameId.push_back(nameId);
        }

        const auto& bindingInfos = GetRC()->textureBindingMgr->BindTextures(needBindingTextures);
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

    void RenderingUtils::BindDrawResources(const RenderParam& renderParam)
    {
        auto shader = renderParam.material->GetShader();
        auto mesh = renderParam.mesh;
        auto material = renderParam.material;
        
        // Bind Shader
        GetRS()->SetShader(shader->glShaderId);
        GL_CHECK_ERROR(BindPredefinedCBuffers)

        // Bind Predefined CBuffers
        auto gMat = GET_GLOBAL_CBUFFER;
        auto vMat = GetGR()->GetPredefinedMaterial(PER_VIEW_CBUFFER);
        // auto oMat = GetGR()->GetPredefinedMaterial(PER_OBJECT_CBUFFER);
        // if (renderParam.localToWorld) { oMat->Set(M, *renderParam.localToWorld);}
        // if (renderParam.worldToLocal) { oMat->Set(IM, *renderParam.worldToLocal);}
        gMat->UseCBuffer();
        vMat->UseCBuffer();
        // oMat->UseCBuffer();
        if (renderParam.objectIndex.has_value())
        {
            GetGR()->perObjectBuffer->Use(renderParam.objectIndex);
            // shader->SetInt(OBJECT_INDEX, static_cast<int>(renderParam.objectIndex.value()));
        }
        GL_CHECK_ERROR(BindPredefinedCBuffers)


        // Bind Vertex Attrib
        GetRS()->BindVertexArray(mesh->vao);
        GetRS()->BindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

        
        // Bind Textures
        ApplyTextures(material, shader);

        
        // Bind CBuffer
        material->UseCBuffer();

        
        // Set Render State
        GetRS()->SetCullMode(material->cullMode);
        GetRS()->SetBlendMode(material->blendMode);
    }
}
