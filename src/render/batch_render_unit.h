#pragma once

#include <cstdint>
#include <memory>
#include <glad/glad.h>

#include "mesh.h"
#include "common/managed_buffer.h"

namespace op
{
    class BatchRenderUnit
    {
    public:
        BatchRenderUnit();
        ~BatchRenderUnit();

        void RegisterMesh(Mesh* mesh);

    private:
        struct MeshInfo
        {
            ManagedBlockId vertexBufferId;
            ManagedBlockId indexBufferId;
        };
        
        GLuint m_vao = GL_NONE;
        GLuint m_vbo = GL_NONE;
        GLuint m_ebo = GL_NONE;

        std::unique_ptr<ManagedBuffer> m_vertexBuffer = nullptr;
        std::unique_ptr<ManagedBuffer> m_indexBuffer = nullptr;
        
        void RecreateResource();
        
        static uint32_t GetMaxVertexDataStride();
    };
}
