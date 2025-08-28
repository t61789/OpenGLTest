#pragma once
#include <cstdint>
#include <memory>
#include <glad/glad.h>

#include "mesh.h"
#include "common/managed_buffer.h"

namespace op
{
    class BatchMesh
    {
    public:
        BatchMesh();
        ~BatchMesh();
        void Use();
        void RegisterMesh(Mesh* mesh);
        void GetMeshInfo(Mesh* mesh, uint32_t& vertexOffset, uint32_t& vertexSize, uint32_t& indexOffset, uint32_t& indexSize);

    private:
        struct MeshInfo
        {
            Mesh* mesh;
            ManagedBlockId vertexBufferId;
            ManagedBlockId indexBufferId;
        };
        
        GLuint m_vao = GL_NONE;
        GLuint m_vbo = GL_NONE;
        GLuint m_ebo = GL_NONE;

        uint32_t m_glVertexBufferCapacity = 0;
        uint32_t m_glIndexBufferCapacity = 0;

        std::unique_ptr<ManagedBuffer> m_vertexBuffer = nullptr;
        std::unique_ptr<ManagedBuffer> m_indexBuffer = nullptr;
        std::vector<MeshInfo> m_meshes;
        std::vector<Mesh*> m_delayAddMeshes;

        void RegisterMeshActually();
        void RecreateGlBuffer(GLuint glTarget, uint32_t capacity, GLuint& glBuffer);
        ManagedBlockId AddDataToLocal(const void* src, uint32_t size, ManagedBuffer* dstBuffer);
        void SyncLocalDataToGl(ManagedBuffer* localBuffer, ManagedBlockId blockId, GLuint glTarget, GLuint& glBufferId,uint32_t& glBufferCapacity);
        
        static const std::vector<float>& GetCompleteVertexData(Mesh* mesh);
    };
}
