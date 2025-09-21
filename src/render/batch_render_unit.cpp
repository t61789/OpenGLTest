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
    bool BatchRenderUnit::PerCmdKey::operator==(const PerCmdKey& rhs) const
    {
        return material == rhs.material && hasONS == rhs.hasONS;
    }

    bool BatchRenderUnit::PerCmdKey::operator!=(const PerCmdKey& rhs) const
    {
        return !(*this == rhs);
    }

    bool BatchRenderUnit::PerSubCmdKey::operator==(const PerSubCmdKey& rhs) const
    {
        return mesh == rhs.mesh;
    }

    bool BatchRenderUnit::PerSubCmdKey::operator!=(const PerSubCmdKey& rhs) const
    {
        return !(*this == rhs);
    }

    BatchRenderUnit::BatchRenderUnit()
    {
        m_cmdBuffer = msp<GlBuffer>(GL_DRAW_INDIRECT_BUFFER);
        m_matrixIndicesBuffer = msp<GlBuffer>(GL_SHADER_STORAGE_BUFFER, 5);
        m_batchMesh = msp<BatchMesh>();
        m_batchMatrix = msp<BatchMatrix>(200, 6);
    }

    void BatchRenderUnit::BindComp(BatchRenderComp* comp)
    {
        if (find(m_comps, &CompInfo::comp, comp) || !comp->GetMesh() || !comp->GetMaterial())
        {
            return;
        }

        m_comps.push_back({m_batchMatrix->Register(), comp});
        m_batchMesh->RegisterMesh(comp->GetMesh());
    }

    void BatchRenderUnit::UnBindComp(const BatchRenderComp* comp)
    {
        auto it = std::find_if(m_comps.begin(), m_comps.end(), [&](const CompInfo& info){ return info.comp == comp;});
        if (it != m_comps.end())
        {
            m_comps.erase(it);
        }
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

        {
            ZoneScopedN("Sort");
            
            std::sort(m_comps.begin(), m_comps.end(), CompComparer);
        }

        static DrawContext drawContext;

        PerCmdKey curPerCmdKey;
        for (auto it = m_comps.begin(); it != m_comps.end(); ++it)
        {
            drawContext.sameMatCompsEnd = it;
            auto& comp = it->comp;

            PerCmdKey perCmdKey = {
                comp->GetMaterial().get(),
                comp->HasONS()
            };
            
            if (perCmdKey != curPerCmdKey)
            {
                if (curPerCmdKey.material)
                {
                    drawContext.material = curPerCmdKey.material;
                    drawContext.hasONS = curPerCmdKey.hasONS;
                    
                    EncodePerMaterialCmds(drawContext);
                }
                curPerCmdKey = perCmdKey;
                drawContext.sameMatCompsBegin = it;
            }
            
            if (it + 1 == m_comps.end() || !(it + 1)->comp->GetInView())
            {
                drawContext.sameMatCompsEnd = m_comps.end();
                
                drawContext.material = curPerCmdKey.material;
                drawContext.hasONS = curPerCmdKey.hasONS;
                
                EncodePerMaterialCmds(drawContext);

                break;
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

        PerSubCmdKey curPerSubCmdKey;
        for (auto it = drawContext.sameMatCompsBegin; it != drawContext.sameMatCompsEnd; ++it)
        {
            auto& comp = it->comp;
            comp->UpdateTransform();

            PerSubCmdKey perSubCmdKey = {
                comp->GetMesh().get()
            };
            
            if (perSubCmdKey != curPerSubCmdKey)
            {
                if (instanceCount != 0)
                {
                    drawContext.mesh = curPerSubCmdKey.mesh;
                    
                    drawContext.cmds.push_back(EncodePerMeshCmd(curPerSubCmdKey.mesh, instanceCount, baseInstanceCount));
                }
                baseInstanceCount += instanceCount;
                instanceCount = 0;

                curPerSubCmdKey = perSubCmdKey;
            }

            instanceCount++;

            if (it + 1 == drawContext.sameMatCompsEnd)
            {
                drawContext.mesh = curPerSubCmdKey.mesh;
                
                drawContext.cmds.push_back(EncodePerMeshCmd(curPerSubCmdKey.mesh, instanceCount, baseInstanceCount));
            }

            drawContext.matrixIndices.push_back(it->matrixIndex);
        }

        CallGlCmd(drawContext);
    }

    void BatchRenderUnit::CallGlCmd(const DrawContext& drawContext)
    {
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

    bool BatchRenderUnit::CompComparer(const CompInfo& lhs, const CompInfo& rhs)
    {
        auto lMat = reinterpret_cast<uintptr_t>(lhs.comp->GetMaterial().get());
        auto lMesh = reinterpret_cast<uintptr_t>(lhs.comp->GetMesh().get());
        auto rMat = reinterpret_cast<uintptr_t>(rhs.comp->GetMaterial().get());
        auto rMesh = reinterpret_cast<uintptr_t>(rhs.comp->GetMesh().get());
        auto lONS = lhs.comp->HasONS();
        auto rONS = rhs.comp->HasONS();
        auto lInView = lhs.comp->GetInView();
        auto rInView = rhs.comp->GetInView();

        if (lInView == rInView)
        {
            if (lMat == rMat)
            {
                if (lONS == rONS)
                {
                    return lMesh < rMesh;
                }
                return lONS < rONS;
            }
            return lMat < rMat;
        }
        return lInView > rInView;
    }
}
