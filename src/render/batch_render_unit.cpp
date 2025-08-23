#include "batch_render_unit.h"

#include "render_state.h"
#include "utils.h"

namespace op
{
    BatchRenderUnit::BatchRenderUnit()
    {
        glGenVertexArrays(1, &m_vao);

        m_vertexBuffer = std::make_unique<ManagedBuffer>(50000000);
        m_indexBuffer = std::make_unique<ManagedBuffer>(100000);
        
        RecreateResource();
    }

    BatchRenderUnit::~BatchRenderUnit()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
    }

    void BatchRenderUnit::RegisterMesh(Mesh* mesh)
    {
        // mesh->
    }

    void BatchRenderUnit::RecreateResource()
    {
        auto vertexDataStride = GetMaxVertexDataStride();

        if (m_vbo != GL_NONE)
        {
            glDeleteBuffers(1, &m_vbo);
            glDeleteBuffers(1, &m_ebo);
        }
        
        GetRS()->BindVertexArray(m_vao);
        
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        GetRS()->BindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_vertexBuffer->Capacity()), nullptr, GL_STATIC_DRAW);

        GetRS()->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_indexBuffer->Capacity()), nullptr, GL_STATIC_DRAW);

        auto offset = 0;
        for (auto& attr : VERTEX_ATTR_DEFINES)
        {
            glEnableVertexAttribArray(static_cast<uint32_t>(attr.attr));
            glVertexAttribPointer(
                static_cast<uint32_t>(attr.attr),
                attr.stride,
                GL_FLOAT,
                GL_FALSE,
                vertexDataStride, reinterpret_cast<const void*>(offset));
            offset += attr.stride * sizeof(float);
        }
    }

    uint32_t BatchRenderUnit::GetMaxVertexDataStride()
    {
        uint32_t size = 0;
        for (auto& attr : VERTEX_ATTR_DEFINES)
        {
            size += attr.stride;
        }
        return size;
    }
}
