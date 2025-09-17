#pragma once

#include <cstdint>
#include <memory>

#include "batch_matrix.h"
#include "utils.h"

namespace op
{
    class BatchMesh0;
    class Mesh;
    class Material;
    class BatchRenderComp;
    
    class BatchRenderUnit final : public Singleton<BatchRenderUnit>
    {
    public:
        BatchRenderUnit();

        void BindComp(BatchRenderComp* comp);
        void UnBindComp(const BatchRenderComp* comp);
        void UpdateMatrix(BatchRenderComp* comp, cr<BatchMatrix::Elem> matrices);
        void Execute();

    private:
        struct IndirectCmd
        {
            uint32_t count;
            uint32_t instanceCount;
            uint32_t firstIndex;
            uint32_t baseVertex;
            uint32_t baseInstance;
        };

        struct CompInfo
        {
            uint32_t matrixIndex;
            BatchRenderComp* comp;
        };

        struct DrawContext
        {
            Material* material = nullptr;
            Mesh* mesh = nullptr;
            
            vec<CompInfo>::iterator sameMatCompsBegin;
            vec<CompInfo>::iterator sameMatCompsEnd;

            vec<IndirectCmd> cmds;
            vec<uint32_t> matrixIndices;
        };

        vec<CompInfo> m_comps;
        
        sp<GlBuffer> m_cmdBuffer = nullptr;
        sp<GlBuffer> m_matrixIndicesBuffer = nullptr;
        sp<BatchMesh0> m_batchMesh = nullptr;
        sp<BatchMatrix> m_batchMatrix = nullptr;

        void EncodePerMaterialCmds(DrawContext& drawContext);
        IndirectCmd EncodePerMeshCmd(Mesh* mesh, uint32_t instanceCount, uint32_t baseInstanceCount);
        void CallGlCmd(const DrawContext& drawContext);

        static bool CompComparer(const CompInfo& lhs, const CompInfo& rhs);
    };
}
