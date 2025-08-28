#include "batch_render_unit.h"

#include "rendering_utils.h"
#include "objects/batch_render_comp.h"
#include "render_state.h"
#include "utils.h"

namespace op
{
    BatchRenderUnit::BatchRenderUnit()
    {
        m_batchMesh = std::make_unique<BatchMesh>();
        m_batchMatrix = std::make_unique<BatchMatrix>(3000, 5);

        glGenBuffers(1, &m_glCmdBuffer);
        glGenBuffers(1, &m_glMatrixIndicesBuffer);
    }

    BatchRenderUnit::~BatchRenderUnit()
    {
        glDeleteBuffers(1, &m_glCmdBuffer);
        glDeleteBuffers(1, &m_glMatrixIndicesBuffer);
    }

    void BatchRenderUnit::BindComp(BatchRenderComp* comp)
    {
        if (find(m_comps, &CompInfo::comp, comp) || !comp->GetMesh() || !comp->GetMaterial())
        {
            return;
        }

        m_comps.push_back({m_batchMatrix->Alloc(), comp});
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

    void BatchRenderUnit::UpdateMatrix(BatchRenderComp* comp, const BatchMatrix::Elem* matrices)
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
        
        std::sort(m_comps.begin(), m_comps.end(), CompComparer);
        
        static DrawContext drawContext;

        Material* curMaterial = nullptr;
        for (auto it = m_comps.begin(); it != m_comps.end(); ++it)
        {
            drawContext.sameMatCompsEnd = it;
            auto& comp = it->comp;
            if (comp->GetMaterial() != curMaterial)
            {
                if (curMaterial)
                {
                    drawContext.material = curMaterial;
                    EncodePerMaterialCmds(drawContext);
                }
                curMaterial = comp->GetMaterial();
                drawContext.sameMatCompsBegin = it;
            }
            
            if (it + 1 == m_comps.end())
            {
                drawContext.sameMatCompsEnd = m_comps.end();
                drawContext.material = curMaterial;
                EncodePerMaterialCmds(drawContext);
            }
        }
    }

    void BatchRenderUnit::EncodePerMaterialCmds(DrawContext& drawContext)
    {
        if (drawContext.sameMatCompsBegin == drawContext.sameMatCompsEnd)
        {
            return;
        }
        
        drawContext.cmds.clear();
        drawContext.matrixIndices.clear();

        uint32_t instanceCount = 0;
        uint32_t baseInstanceCount = 0;
        Mesh* curMesh = nullptr;
        for (auto it = drawContext.sameMatCompsBegin; it != drawContext.sameMatCompsEnd; ++it)
        {
            auto& comp = it->comp;
            comp->UpdateTransform();
            
            if (comp->GetMesh() != curMesh)
            {
                if (instanceCount != 0)
                {
                    drawContext.mesh = curMesh;
                    drawContext.cmds.push_back(EncodePerMeshCmd(curMesh, instanceCount, baseInstanceCount));
                }
                baseInstanceCount += instanceCount;
                instanceCount = 1;
                curMesh = comp->GetMesh();
            }

            if (it + 1 == drawContext.sameMatCompsEnd)
            {
                drawContext.mesh = curMesh;
                drawContext.cmds.push_back(EncodePerMeshCmd(curMesh, instanceCount, baseInstanceCount));
            }

            drawContext.matrixIndices.push_back(it->matrixIndex);
        }

        CallGlCmd(drawContext);
    }

    void BatchRenderUnit::CallGlCmd(const DrawContext& drawContext)
    {
        GetRS()->BindBuffer(GL_DRAW_INDIRECT_BUFFER, m_glCmdBuffer);
        glBufferData(
            GL_DRAW_INDIRECT_BUFFER,
            static_cast<GLsizeiptr>(sizeof(IndirectCmd) * drawContext.cmds.size()),
            drawContext.cmds.data(),
            GL_STREAM_DRAW);

        GetRS()->BindBuffer(GL_SHADER_STORAGE_BUFFER, m_glMatrixIndicesBuffer);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            static_cast<GLsizeiptr>(sizeof(uint32_t) * drawContext.matrixIndices.size()),
            drawContext.matrixIndices.data(),
            GL_STREAM_DRAW);
        GetRS()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_glMatrixIndicesBuffer);
        m_batchMatrix->Use();
        m_batchMesh->Use();
        
        RenderingUtils::BindDrawResources({
            nullptr,
            drawContext.material,
            nullptr,
            nullptr
        });

        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(drawContext.cmds.size()), 0);
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
        auto lMat = reinterpret_cast<uintptr_t>(lhs.comp->GetMaterial());
        auto lMesh = reinterpret_cast<uintptr_t>(lhs.comp->GetMesh());
        auto rMat = reinterpret_cast<uintptr_t>(rhs.comp->GetMaterial());
        auto rMesh = reinterpret_cast<uintptr_t>(rhs.comp->GetMesh());
        if (lMat == rMat)
        {
            return lMesh < rMesh;
        }
        return lMat < rMat;
    }
}
