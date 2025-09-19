#pragma once
#include <cstdint>
#include <memory>

#include "utils.h"
#include "common/elem_accessor_var.h"

namespace op
{
    class Mesh;
    class GlVertexArray;
    class GlSubmitBuffer;
    
    class BatchMesh final
    {
    public:
        BatchMesh();

        void Use();
        void StopUse();
        void RegisterMesh(crsp<Mesh> mesh);
        void GetMeshInfo(Mesh* mesh, uint32_t& vertexOffsetB, uint32_t& vertexSizeB, uint32_t& indexOffsetB, uint32_t& indexSizeB);

    private:
        struct MeshInfo
        {
            sp<Mesh> mesh;
            size_t vertexBufferId;
            size_t indexBufferId;
        };

        sp<GlVertexArray> m_vao;
        sp<GlSubmitBuffer> m_vbo;
        sp<GlSubmitBuffer> m_ebo;

        up<ElemAccessorVar> m_vboAccessor;
        up<ElemAccessorVar> m_eboAccessor;

        vec<MeshInfo> m_meshes;
        vecwp<Mesh> m_delayAddMeshes;

        void RegisterMeshActually();
        
        static void ConfigVaoPointer(crsp<GlVertexArray> vao);
        static crvec<float> GetFullVertexData(crwp<Mesh> meshPtr);
    };
}
