#pragma once

#include <cstdint>
#include <memory>

#include <boost/lockfree/queue.hpp>

#include "batch_matrix.h"
#include "utils.h"
#include "common/managed_buffer.h"

namespace op
{
    class BatchMesh;
    class Mesh;
    class Material;
    class BatchRenderComp;
    
    class BatchRenderUnit
    {
    public:
        BatchRenderUnit();

        void BindComp(BatchRenderComp* comp);
        void UnBindComp(BatchRenderComp* comp);
        void UpdateMatrix(BatchRenderComp* comp, cr<BatchMatrix::Elem> matrices);
        void Execute();
        void StartEncodingCmds();

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

        struct Cmd;

        struct SubCmd
        {
            Mesh* mesh = nullptr;
            
            IndirectCmd indirectCmd = {};
            
            vec<CompInfo*> comps = {};
        };

        struct Cmd
        {
            Material* material = nullptr;
            bool hasONS = true;

            vec<IndirectCmd> indirectCmds = {};
            vec<uint32_t> matrixIndices = {};
            
            vec<SubCmd*> subCmds = {};
        };

        sp<GlBuffer> m_cmdBuffer = nullptr;
        sp<GlBuffer> m_matrixIndicesBuffer = nullptr;
        sp<BatchMesh> m_batchMesh = nullptr;
        sp<BatchMatrix> m_batchMatrix = nullptr;

        vec<Cmd*> m_cmds;
        umap<BatchRenderComp*, CompInfo*> m_comps;
        
        lock_free_queue<Cmd*> m_encodedCmds = lock_free_queue<Cmd*>(1024);

        bool m_encodingCmds = false;

        void CallGlCmd(const Cmd* cmd);
        void AddComp(BatchRenderComp* comp);
        void RemoveComp(BatchRenderComp* comp);
        void EncodeCmdsTask();
        
        IndirectCmd CreateIndirectCmd(Mesh* mesh);
    };
}
