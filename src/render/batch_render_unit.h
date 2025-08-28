#pragma once

#include <cstdint>
#include <memory>
#include <glad/glad.h>

#include "batch_matrix.h"
#include "batch_mesh.h"
#include "mesh.h"
#include "material.h"
#include "common/managed_buffer.h"

namespace op
{
    class BatchRenderComp;
    
    class BatchRenderUnit : public Singleton<BatchRenderUnit>
    {
    public:
        BatchRenderUnit();
        ~BatchRenderUnit();

        void BindComp(BatchRenderComp* comp);
        void UnBindComp(const BatchRenderComp* comp);
        void UpdateMatrix(BatchRenderComp* comp, const BatchMatrix::Elem* matrices);
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
            
            std::vector<CompInfo>::iterator sameMatCompsBegin;
            std::vector<CompInfo>::iterator sameMatCompsEnd;

            std::vector<IndirectCmd> cmds;
            std::vector<uint32_t> matrixIndices;
        };

        std::vector<CompInfo> m_comps;
        std::unique_ptr<BatchMesh> m_batchMesh = nullptr;
        std::unique_ptr<BatchMatrix> m_batchMatrix = nullptr;

        GLuint m_glCmdBuffer;
        GLuint m_glMatrixIndicesBuffer;

        void EncodePerMaterialCmds(DrawContext& drawContext);
        IndirectCmd EncodePerMeshCmd(Mesh* mesh, uint32_t instanceCount, uint32_t baseInstanceCount);
        void CallGlCmd(const DrawContext& drawContext);

        static bool CompComparer(const CompInfo& lhs, const CompInfo& rhs);
    };
}
