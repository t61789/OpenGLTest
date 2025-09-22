#include "batch_render_unit.h"

#include <tracy/Tracy.hpp>

#include "batch_matrix.h"
#include "batch_mesh.h"
#include "rendering_utils.h"
#include "objects/batch_render_comp.h"
#include "utils.h"
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
        if (!comp->GetMesh() || !comp->GetMaterial())
        {
            return;
        }

        m_batchMesh->RegisterMesh(comp->GetMesh());

        AddToCompsOrderly(comp);
    }

    void BatchRenderUnit::UnBindComp(const BatchRenderComp* comp)
    {
        remove_if(m_comps, [&comp](cr<CompInfo> info)
        {
            return info.comp == comp;
        });
    }

    void BatchRenderUnit::UpdateMatrix(BatchRenderComp* comp, cr<BatchMatrix::Elem> matrices)
    {
        auto compInfo = find(m_comps, &CompInfo::comp, comp);
        m_batchMatrix->SubmitData(compInfo->matrixIndex, matrices);
    }

    void BatchRenderUnit::Execute()
    {
        if (m_comps.empty())
        {
            return;
        }

        static DrawContext drawContext;

        std::tuple<Material*, bool> curPerCmdKey;
        for (auto it = m_comps.begin();; ++it)
        {
            drawContext.sameMatCompsEnd = it;
            
            if (it == m_comps.end())
            {
                std::tie(drawContext.material, drawContext.hasONS) = curPerCmdKey;
                EncodePerMaterialCmds(drawContext);

                break;
            }
            
            auto& comp = it->comp;
            auto perCmdKey = std::tuple(comp->GetMaterial().get(), comp->HasONS()); // todo make them const
            if (perCmdKey != curPerCmdKey)
            {
                if (it != m_comps.begin())
                {
                    std::tie(drawContext.material, drawContext.hasONS) = curPerCmdKey;
                    EncodePerMaterialCmds(drawContext);
                }
                
                curPerCmdKey = perCmdKey;
                drawContext.sameMatCompsBegin = it;
            }
        }
    }

    void BatchRenderUnit::EncodePerMaterialCmds(DrawContext& drawContext)
    {
        if (drawContext.sameMatCompsBegin == drawContext.sameMatCompsEnd)
        {
            return;
        }

        ZoneScoped;
        
        drawContext.cmds.clear();
        drawContext.matrixIndices.clear();

        uint32_t instanceCount = 0;
        uint32_t baseInstanceCount = 0;

        std::tuple<Mesh*> curPerSubCmdKey;
        for (auto it = drawContext.sameMatCompsBegin;; ++it)
        {
            if (it == drawContext.sameMatCompsEnd)
            {
                if (instanceCount != 0)
                {
                    auto cmd = EncodePerMeshCmd(std::get<0>(curPerSubCmdKey), instanceCount, baseInstanceCount);
                    drawContext.cmds.push_back(cmd);
                }

                break;
            }
            
            auto& comp = it->comp;
            if (!comp->GetInView())
            {
                // continue;
            }

            auto perSubCmdKey = std::tuple(comp->GetMesh().get());
            if (perSubCmdKey != curPerSubCmdKey)
            {
                if (instanceCount != 0)
                {
                    auto cmd = EncodePerMeshCmd(std::get<0>(curPerSubCmdKey), instanceCount, baseInstanceCount);
                    drawContext.cmds.push_back(cmd);
                }
                
                baseInstanceCount += instanceCount;
                instanceCount = 0;

                curPerSubCmdKey = perSubCmdKey;
            }

            instanceCount++;
            drawContext.matrixIndices.push_back(it->matrixIndex);
        }

        CallGlCmd(drawContext);
    }

    void BatchRenderUnit::CallGlCmd(const DrawContext& drawContext)
    {
        if (drawContext.cmds.empty())
        {
            return;
        }
        
        m_cmdBuffer->Bind();
        m_cmdBuffer->SetData(
            GL_STREAM_DRAW,
            sizeof(IndirectCmd) * drawContext.cmds.size(),
            drawContext.cmds.data());

        m_matrixIndicesBuffer->Bind();
        m_matrixIndicesBuffer->SetData(
            GL_STREAM_DRAW,
            sizeof(uint32_t) * drawContext.matrixIndices.size(),
            drawContext.matrixIndices.data());
        m_matrixIndicesBuffer->BindBase();
        
        m_batchMatrix->Use();
        m_batchMesh->Use();
        
        RenderingUtils::BindDrawResources({
            nullptr,
            drawContext.material,
            drawContext.hasONS
        });

        GlState::GlMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(drawContext.cmds.size()), 0);
    }

    BatchRenderUnit::IndirectCmd BatchRenderUnit::EncodePerMeshCmd(Mesh* mesh, const uint32_t instanceCount, const uint32_t baseInstanceCount)
    {
        uint32_t vertexOffset, vertexSize, indexOffset, indexSize;
        m_batchMesh->GetMeshInfo(mesh, vertexOffset, vertexSize, indexOffset, indexSize);
        auto cmd = IndirectCmd();
        cmd.count = static_cast<uint32_t>(indexSize / sizeof(uint32_t));
        cmd.instanceCount = instanceCount;
        cmd.firstIndex = static_cast<uint32_t>(indexOffset / sizeof(uint32_t));
        cmd.baseVertex = static_cast<uint32_t>(vertexOffset / (MAX_VERTEX_ATTR_STRIDE_F * sizeof(float)));
        cmd.baseInstance = baseInstanceCount;

        return cmd;
    }

    void BatchRenderUnit::AddToCompsOrderly(BatchRenderComp* comp)
    {
        auto sortKey = std::tuple(
            comp->GetMaterial().get(),
            comp->HasONS(),
            comp->GetMesh().get());
        
        CompInfo compInfo = {
            m_batchMatrix->Register(),
            sortKey,
            comp};

        insert(m_comps, compInfo, [&sortKey](cr<CompInfo> x)
        {
            return x.sortKey < sortKey;
        });
    }
}
