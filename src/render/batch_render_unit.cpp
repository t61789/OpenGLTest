#include "batch_render_unit.h"

#include <tracy/Tracy.hpp>

#include "batch_matrix.h"
#include "batch_mesh.h"
#include "game_resource.h"
#include "material.h"
#include "rendering_utils.h"
#include "shader.h"
#include "objects/batch_render_comp.h"
#include "utils.h"
#include "common/asset_cache.h"
#include "gl/gl_buffer.h"
#include "gl/gl_state.h"

namespace op
{
    BatchRenderUnit::BatchRenderUnit()
    {
        m_cmdBuffer = msp<GlBuffer>(GL_DRAW_INDIRECT_BUFFER);
        m_matrixIndicesBuffer = msp<GlBuffer>(GL_SHADER_STORAGE_BUFFER, 5);
        m_batchMesh = msp<BatchMesh>();
        m_batchMatrix = msp<BatchMatrix>(200, 6);
    }

    void BatchRenderUnit::BindComp(BatchRenderComp* comp)
    {
        AddComp(comp);
    }

    void BatchRenderUnit::UnBindComp(BatchRenderComp* comp)
    {
        RemoveComp(comp);
    }

    void BatchRenderUnit::UpdateMatrix(BatchRenderComp* comp, cr<BatchMatrix::Elem> matrices)
    {
        auto compInfo = m_comps.find(comp);
        assert(compInfo != m_comps.end());
        m_batchMatrix->SubmitData(compInfo->second->matrixIndex, matrices);
    }
    
    void BatchRenderUnit::Execute()
    {
        assert(m_encodingCmds);
        
        m_batchMatrix->Use();
        m_batchMesh->Use();
        
        GetGlobalCbuffer()->BindBase();
        GetPerViewCbuffer()->BindBase();

        DrawContext context;
        
        while (true)
        {
            Cmd* cmd = nullptr;
            if (!m_encodedCmds.pop(cmd))
            {
                std::this_thread::yield();
                continue;
            }

            if (cmd)
            {
                CallGlCmd(cmd, context);
            }
            else
            {
                break;
            }
        }

        m_encodingCmds = false;
    }

    void BatchRenderUnit::StartEncodingCmds()
    {
        assert(!m_encodingCmds && m_encodedCmds.empty());

        m_encodingCmds = true;

        GetGR()->GetThreadPool()->Start([this]
        {
            this->EncodeCmdsTask();
        });
    }

    void BatchRenderUnit::AddComp(BatchRenderComp* comp)
    {
        auto material = comp->GetMaterial().get();
        auto mesh = comp->GetMesh();
        auto hasONS = comp->HasONS();

        if (mesh == nullptr || material == nullptr)
        {
            return;
        }
        
        auto cmd = op::find_if(m_cmds, [material, hasONS](const Cmd* x)
        {
            return x->material == material && x->hasONS == hasONS;
        });
        if (!cmd)
        {
            m_cmds.push_back(new Cmd{
                material,
                hasONS,
            });
            cmd = m_cmds.back();
        }

        
        auto subCmd = find_if(cmd->subCmds, [mesh=mesh.get()](const SubCmd* x)
        {
            return x->mesh == mesh;
        });
        if (!subCmd)
        {
            m_batchMesh->RegisterMesh(mesh);
            cmd->subCmds.push_back(new SubCmd{
                mesh.get(),
                CreateIndirectCmd(mesh.get())
            });
            subCmd = cmd->subCmds.back();
        }

        
        assert(!exists_if(subCmd->comps, [comp](const CompInfo* x){ return x->comp == comp; }));
        subCmd->comps.push_back(new CompInfo{
            m_batchMatrix->Register(),
            comp
        });
        m_comps[comp] = subCmd->comps.back();
        cmd->compCount++;
    }

    void BatchRenderUnit::RemoveComp(BatchRenderComp* comp)
    {
        auto material = comp->GetMaterial().get();
        auto mesh = comp->GetMesh().get();
        auto hasONS = comp->HasONS();
        
        if (mesh == nullptr || material == nullptr)
        {
            return;
        }
        
        auto cmd = find_if(m_cmds, [material, hasONS](const Cmd* x)
        {
            return x->material == material && x->hasONS == hasONS;
        });
        if (cmd == nullptr)
        {
            return;
        }
        

        auto subCmd = find(cmd->subCmds, &SubCmd::mesh, mesh);
        if (subCmd == nullptr)
        {
            return;
        }

        
        auto compInfo = find(subCmd->comps, &CompInfo::comp, comp);
        if (compInfo == nullptr)
        {
            return;
        }
        

        cmd->compCount--;
        m_batchMatrix->UnRegister(compInfo->matrixIndex);
        remove(subCmd->comps, compInfo);
        m_comps.erase(comp);
        delete compInfo;
        if (subCmd->comps.empty())
        {
            remove(cmd->subCmds, subCmd);
            delete subCmd;
        }
        if (cmd->subCmds.empty())
        {
            remove(m_cmds, cmd);
            delete cmd;
        }
    }

    void BatchRenderUnit::EncodeCmdsTask()
    {
        ZoneScoped;
        
        for (auto cmd : m_cmds)
        {
            ZoneScopedN("Encode Cmd");
            
            cmd->indirectCmds.Clear();
            cmd->indirectCmds.Reserve(cmd->subCmds.size());
            cmd->matrixIndices.Clear();
            cmd->matrixIndices.Reserve(cmd->compCount);

            auto baseInstanceCount = 0;
            for (auto& subCmd : cmd->subCmds)
            {
                auto instanceCount = 0;
                for (auto& compInfo : subCmd->comps)
                {
                    // if (compInfo->comp->GetInView())
                    // {
                        cmd->matrixIndices.Add<false>(compInfo->matrixIndex);
                        instanceCount++;
                    // }
                }

                subCmd->indirectCmd.instanceCount = instanceCount;
                subCmd->indirectCmd.baseInstance = baseInstanceCount;

                baseInstanceCount += instanceCount;

                if (instanceCount != 0)
                {
                    cmd->indirectCmds.Add<false>(subCmd->indirectCmd);
                }
            }

            if (!cmd->indirectCmds.Empty())
            {
                this->m_encodedCmds.push(cmd);
            }
        }

        this->m_encodedCmds.push(nullptr);
    }
    
    BatchRenderUnit::IndirectCmd BatchRenderUnit::CreateIndirectCmd(Mesh* mesh)
    {
        uint32_t vertexOffset, vertexSize, indexOffset, indexSize;
        m_batchMesh->GetMeshInfo(mesh, vertexOffset, vertexSize, indexOffset, indexSize);
        auto cmd = IndirectCmd();
        cmd.count = static_cast<uint32_t>(indexSize / sizeof(uint32_t));
        cmd.instanceCount = 0;
        cmd.firstIndex = static_cast<uint32_t>(indexOffset / sizeof(uint32_t));
        cmd.baseVertex = static_cast<uint32_t>(vertexOffset / (MAX_VERTEX_ATTR_STRIDE_F * sizeof(float)));
        cmd.baseInstance = 0;

        return cmd;
    }

    void BatchRenderUnit::CallGlCmd(const Cmd* cmd, DrawContext& context)
    {
        ZoneScoped;

        auto shaderChanged = context.shader != cmd->material->GetShader().get();
        auto materialChanged = context.material != cmd->material;
        auto onsChanged = cmd->hasONS != context.hasONS;
        auto textureSetHashChanged = cmd->material->GetTextureSet()->GetHash() != context.textureSetHash;

        context.shader = cmd->material->GetShader().get();
        context.material = cmd->material;
        context.hasONS = cmd->hasONS;
        context.textureSetHash = cmd->material->GetTextureSet()->GetHash();

        if (shaderChanged)
        {
            context.shader->Use();
        }

        if (shaderChanged || textureSetHashChanged)
        {
            cmd->material->GetTextureSet()->ApplyTextures(context.shader);
        }

        if (materialChanged || onsChanged)
        {
            GlState::Ins()->SetCullMode(context.material->cullMode, context.hasONS);
        }

        if (materialChanged)
        {
            context.material->UseCBuffer();
            
            GlState::Ins()->SetBlendMode(context.material->blendMode);
            GlState::Ins()->SetDepthMode(context.material->depthMode);
        }

        {
            ZoneScopedN("Submit GL Draw Data");
            
            m_cmdBuffer->Bind();
            m_cmdBuffer->SetData(
                GL_STREAM_DRAW,
                sizeof(IndirectCmd) * cmd->indirectCmds.Size(),
                cmd->indirectCmds.Data());

            m_matrixIndicesBuffer->Bind();
            m_matrixIndicesBuffer->SetData(
                GL_STREAM_DRAW,
                sizeof(uint32_t) * cmd->matrixIndices.Size(),
                cmd->matrixIndices.Data());
            m_matrixIndicesBuffer->BindBase();
        }
        
        GlState::GlMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(cmd->indirectCmds.Size()), 0);
    }
}
