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
    class BatchRenderUnit;
    
    BatchRenderUnit::IndirectCmd BatchRenderUnit::IndirectCmd::CreateIndirectCmd(cr<BatchRenderParam> param)
    {
        uint32_t vertexOffset, vertexSize, indexOffset, indexSize;
        param.unit->m_batchMesh->GetMeshInfo(
            param.mesh.get(),
            vertexOffset,
            vertexSize,
            indexOffset,
            indexSize);
        
        auto cmd = IndirectCmd();
        cmd.count = static_cast<uint32_t>(indexSize / sizeof(uint32_t));
        cmd.instanceCount = 0;
        cmd.firstIndex = static_cast<uint32_t>(indexOffset / sizeof(uint32_t));
        cmd.baseVertex = static_cast<uint32_t>(vertexOffset / (MAX_VERTEX_ATTR_STRIDE_F * sizeof(float)));
        cmd.baseInstance = 0;

        return cmd;
    }

    BatchRenderUnit::BatchRenderCompInfo* BatchRenderUnit::BatchRenderTree::AddComp(cr<BatchRenderParam> param)
    {
        auto material = param.material;
        auto mesh = param.mesh;
        auto comp = param.comp;
        auto hasONS = param.hasONS;

        assert(material && mesh && comp);

        // find cmd
        auto cmd = op::find_if(cmds, [material, hasONS](const BatchRenderCmd* x)
        {
            return x->material == material && x->hasONS == hasONS;
        });
        if (!cmd)
        {
            cmds.push_back(new BatchRenderCmd{
                material,
                hasONS,
            });
            cmd = cmds.back();
        }

        // find subCmd
        auto subCmd = find_if(cmd->subCmds, [mesh](const BatchRenderSubCmd* x)
        {
            return x->mesh == mesh.get();
        });
        if (!subCmd)
        {
            param.unit->m_batchMesh->RegisterMesh(comp->GetMesh());
            cmd->subCmds.push_back(new BatchRenderSubCmd{
                mesh.get(),
                IndirectCmd::CreateIndirectCmd(param)
            });
            subCmd = cmd->subCmds.back();
        }

        // find comp
        assert(!exists_if(subCmd->comps, [comp](const BatchRenderCompInfo* x){ return x->comp == comp; }));
        subCmd->comps.push_back(new BatchRenderCompInfo{
            param.matrixIndex,
            comp
        });
        cmd->compCount++;

        return subCmd->comps.back();
    }

    void BatchRenderUnit::BatchRenderTree::RemoveComp(BatchRenderComp* comp)
    {
        auto removeFromSubCmd = [comp](BatchRenderSubCmd* subCmd)
        {
            if (auto compInfo = find(subCmd->comps, &BatchRenderCompInfo::comp, comp))
            {
                remove(subCmd->comps, compInfo);
                delete compInfo;
                return true;
            }

            return false;
        };

        auto removeFromCmd = [removeFromSubCmd](BatchRenderCmd* cmd)
        {
            if (auto subCmd = find_if(cmd->subCmds, removeFromSubCmd))
            {
                cmd->compCount--;

                if (subCmd->comps.empty())
                {
                    remove(cmd->subCmds, subCmd);
                    delete subCmd;
                }

                return true;
            }

            return false;
        };

        if (auto cmd = find_if(cmds, removeFromCmd))
        {
            if (cmd->subCmds.empty())
            {
                remove(cmds, cmd);
                delete cmd;
            }
        }
    }

    BatchRenderUnit::BatchRenderUnit()
    {
        m_cmdBuffer = msp<GlBuffer>(GL_DRAW_INDIRECT_BUFFER);
        m_matrixIndicesBuffer = msp<GlBuffer>(GL_SHADER_STORAGE_BUFFER, 5);
        m_batchMesh = msp<BatchMesh>();
        m_batchMatrix = msp<BatchMatrix>(200, 6);

        for (auto i = 0; i < static_cast<int>(ViewGroup::COUNT); ++i)
        {
            auto renderTree = new BatchRenderTree{
                static_cast<ViewGroup>(i),
                {},
                nullptr,
                lock_free_queue<BatchRenderCmd*>(1024)
            };
            m_renderTrees.emplace_back(renderTree);
        }
    }

    BatchRenderUnit::~BatchRenderUnit()
    {
        assert(m_comps.empty());
    }

    void BatchRenderUnit::UpdateMatrix(BatchRenderComp* comp, cr<BatchMatrix::Elem> matrices)
    {
        auto compInfo = m_comps.find(comp);
        assert(compInfo != m_comps.end());
        m_batchMatrix->SubmitData(compInfo->second, matrices);
    }
    
    void BatchRenderUnit::Execute(const ViewGroup viewGroup)
    {
        auto renderTree = m_renderTrees[static_cast<uint8_t>(viewGroup)].get();
        
        m_batchMatrix->Use();
        m_batchMesh->Use();
        
        GetGlobalCbuffer()->BindBase();
        GetPerViewCbuffer()->BindBase();

        renderTree->encodingJob->WaitForStart();

        DrawContext context;
        
        while (true)
        {
            BatchRenderCmd* cmd = nullptr;
            if (!renderTree->encodedCmds.pop(cmd))
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

        renderTree->encodingJob.reset();
    }

    sp<Job> BatchRenderUnit::CreateEncodingJob(ViewGroup viewGroup)
    {
        auto renderTree = m_renderTrees[static_cast<uint8_t>(viewGroup)].get();
        
        auto job = Job::CreateCommon([this, renderTree]
        {
            renderTree->EncodeCmdsTask();
        });

        renderTree->encodingJob = job;

        return job;
    }

    void BatchRenderUnit::BatchRenderTree::EncodeCmdsTask()
    {
        ZoneScoped;

        BatchRenderCmd* dummy;
        while (encodedCmds.pop(dummy)) {}

        for (auto cmd : cmds)
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
                    if (compInfo->comp->GetInView(viewGroup))
                    {
                        cmd->matrixIndices.Add<false>(compInfo->matrixIndex);
                        instanceCount++;
                    }
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
                encodedCmds.push(cmd);
            }
        }

        encodedCmds.push(nullptr);
    }

    void BatchRenderUnit::BindComp(BatchRenderComp* comp)
    {
        if (m_comps.find(comp) != m_comps.end())
        {
            return;
        }
        auto matrixIndex = m_batchMatrix->Register();
        m_comps[comp] = matrixIndex;

        auto commonRenderTree = m_renderTrees[static_cast<uint8_t>(ViewGroup::COMMON)].get();
        commonRenderTree->AddComp({
            this,
            comp,
            comp->GetMaterial().get(),
            comp->GetMesh(),
            comp->HasONS(),
            matrixIndex
        });

        auto shadowRenderTree = m_renderTrees[static_cast<uint8_t>(ViewGroup::SHADOW)].get();
        shadowRenderTree->AddComp({
            this,
            comp,
            comp->GetMaterial().get(), // TODO
            comp->GetMesh(),
            comp->HasONS(),
            matrixIndex
        });
    }

    void BatchRenderUnit::UnBindComp(BatchRenderComp* comp)
    {
        if (m_comps.find(comp) == m_comps.end())
        {
            return;
        }
        m_comps.erase(comp);

        auto commonRenderTree = m_renderTrees[static_cast<uint8_t>(ViewGroup::COMMON)].get();
        commonRenderTree->RemoveComp(comp);

        auto shadowRenderTree = m_renderTrees[static_cast<uint8_t>(ViewGroup::SHADOW)].get();
        shadowRenderTree->RemoveComp(comp);
    }

    void BatchRenderUnit::CallGlCmd(const BatchRenderCmd* cmd, DrawContext& context)
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
