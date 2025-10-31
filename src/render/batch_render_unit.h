#pragma once

#include <cstdint>
#include <memory>

#include <boost/lockfree/queue.hpp>

#include "batch_matrix.h"
#include "culling_system.h"
#include "utils.h"
#include "common/simple_list.h"
#include "common/tree_node.h"
#include "job_system/job_scheduler.h"

namespace op
{
    class Shader;
    class BatchMesh;
    class Mesh;
    class Material;
    class BatchRenderComp;
    
    class BatchRenderUnit
    {
        struct BatchRenderParam;
        struct IndirectCmd;
        struct BatchRenderCompInfo;
        struct BatchRenderSubCmd;
        struct BatchRenderCmd;
        struct BatchRenderTree;
        
    public:
        BatchRenderUnit();
        ~BatchRenderUnit();
        BatchRenderUnit(const BatchRenderUnit& other) = delete;
        BatchRenderUnit(BatchRenderUnit&& other) noexcept = delete;
        BatchRenderUnit& operator=(const BatchRenderUnit& other) = delete;
        BatchRenderUnit& operator=(BatchRenderUnit&& other) noexcept = delete;

        void BindComp(BatchRenderComp* comp);
        void UnBindComp(BatchRenderComp* comp);
        void UpdateMatrix(BatchRenderComp* comp, cr<BatchMatrix::Elem> matrices);
        void Execute(ViewGroup viewGroup);
        sp<JobScheduler::Job> CreateEncodingJob(ViewGroup viewGroup);

    private:

        struct IndirectCmd
        {
            uint32_t count;
            uint32_t instanceCount;
            uint32_t firstIndex;
            uint32_t baseVertex;
            uint32_t baseInstance;
            
            static IndirectCmd CreateIndirectCmd(cr<BatchRenderParam> param);
        };

        struct BatchRenderParam
        {
            BatchRenderUnit* unit = nullptr;
            BatchRenderComp* comp = nullptr;
            Material* material = nullptr;
            sp<Mesh> mesh = nullptr;
            bool hasONS = true;
            uint32_t matrixIndex = ~0u;
        };
            
        struct BatchRenderCompInfo
        {
            uint32_t matrixIndex;
            BatchRenderComp* comp;
        };
        
        struct BatchRenderSubCmd
        {
            Mesh* mesh = nullptr;
            
            IndirectCmd indirectCmd = {};
            
            vec<BatchRenderCompInfo*> comps = {};
        };

        struct BatchRenderCmd
        {
            Material* material = nullptr;
            bool hasONS = true;
            
            uint32_t compCount = 0;
            SimpleList<IndirectCmd> indirectCmds = {};
            SimpleList<uint32_t> matrixIndices = {};
            
            vec<BatchRenderSubCmd*> subCmds = {};
        };

        struct BatchRenderTree
        {
            ViewGroup viewGroup;
            vec<BatchRenderCmd*> cmds;
            sp<JobScheduler::Job> encodingJob;
            lock_free_queue<BatchRenderCmd*> encodedCmds;

            BatchRenderCompInfo* AddComp(cr<BatchRenderParam> param);
            void RemoveComp(BatchRenderComp* comp);
            void EncodeCmdsTask();
        };

        struct DrawContext
        {
            Shader* shader = nullptr;
            Material* material = nullptr;
            Mesh* mesh = nullptr;
            bool hasONS = true;
            size_t textureSetHash = 0;
        };

        sp<GlBuffer> m_cmdBuffer = nullptr;
        sp<GlBuffer> m_matrixIndicesBuffer = nullptr;
        sp<BatchMesh> m_batchMesh = nullptr;
        sp<BatchMatrix> m_batchMatrix = nullptr;
        umap<BatchRenderComp*, uint32_t> m_comps;
        
        vecup<BatchRenderTree> m_renderTrees;

        void CallGlCmd(const BatchRenderCmd* cmd, DrawContext& context);
        
        friend struct IndirectCmd;
    };
}
