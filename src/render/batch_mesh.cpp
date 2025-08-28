#include "batch_mesh.h"

#include "render_state.h"

namespace op
{
    BatchMesh::BatchMesh()
    {
        glGenVertexArrays(1, &m_vao);

        m_vertexBuffer = std::make_unique<ManagedBuffer>(50000000);
        m_indexBuffer = std::make_unique<ManagedBuffer>(100000);

        RecreateGlBuffer(GL_ARRAY_BUFFER, m_vertexBuffer->Capacity(), m_vbo);
        RecreateGlBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer->Capacity(), m_ebo);

        m_glVertexBufferCapacity = m_vertexBuffer->Capacity();
        m_glIndexBufferCapacity = m_indexBuffer->Capacity();
    }

    BatchMesh::~BatchMesh()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);

        for (auto mesh : m_delayAddMeshes)
        {
            DECREF(mesh);
        }

        for (auto mesh : m_meshes)
        {
            DECREF(mesh.mesh);
        }
    }

    void BatchMesh::Use()
    {
        // float vertex[96];
        // uint32_t vertexOffset, vertexSize;
        // m_vertexBuffer->GetBlockInfo(m_meshes[1].vertexBufferId, vertexOffset, vertexSize);
        // memcpy(vertex, m_vertexBuffer->Data() + vertexOffset, vertexSize);
        
        GetRS()->BindVertexArray(m_vao);
        // GetRS()->BindBuffer(GL_ARRAY_BUFFER, m_vbo);
    }

    void BatchMesh::RegisterMesh(Mesh* mesh)
    {
        m_delayAddMeshes.push_back(mesh);
        INCREF(mesh);
    }

    void BatchMesh::GetMeshInfo(Mesh* mesh, uint32_t& vertexOffset, uint32_t& vertexSize, uint32_t& indexOffset, uint32_t& indexSize)
    {
        RegisterMeshActually();
        
        auto meshInfo = find(m_meshes, &MeshInfo::mesh, mesh);
        if (!meshInfo)
        {
            THROW_ERROR("Unregistered mesh!")
        }
        
        m_vertexBuffer->GetBlockInfo(meshInfo->vertexBufferId, vertexOffset, vertexSize);
        m_indexBuffer->GetBlockInfo(meshInfo->indexBufferId, indexOffset, indexSize);
    }
    
    void BatchMesh::RegisterMeshActually()
    {
        if (m_delayAddMeshes.empty())
        {
            return;
        }
        
        std::vector<Mesh*> needAddMeshes;
        size_t addVertexDataSize = 0;
        size_t addIndexDataSize = 0;
        for (auto mesh : m_delayAddMeshes)
        {
            if (!find(m_meshes, &MeshInfo::mesh, mesh) && !exists(needAddMeshes, mesh))
            {
                INCREF(mesh);
                needAddMeshes.push_back(mesh);
                addVertexDataSize += MAX_VERTEX_ATTR_STRIDE_F * mesh->vertexCount * sizeof(float);
                addIndexDataSize += mesh->indicesCount * sizeof(uint32_t);
            }
            DECREF(mesh);
        }
        m_delayAddMeshes.clear();

        m_vertexBuffer->Reserve(m_vertexBuffer->Size() + addVertexDataSize);
        m_indexBuffer->Reserve(m_indexBuffer->Size() + addIndexDataSize);

        for (auto mesh : needAddMeshes)
        {
            auto& vertexData = GetCompleteVertexData(mesh);
            auto& indexData = mesh->GetIndexData();

            auto vertexBufferId = AddDataToLocal(vertexData.data(), vertexData.size() * sizeof(float), m_vertexBuffer.get());
            SyncLocalDataToGl(m_vertexBuffer.get(), vertexBufferId, GL_ARRAY_BUFFER, m_vbo, m_glVertexBufferCapacity);
            
            auto indexBufferId = AddDataToLocal(indexData.data(), indexData.size() * sizeof(uint32_t), m_indexBuffer.get());
            SyncLocalDataToGl(m_indexBuffer.get(), indexBufferId, GL_ELEMENT_ARRAY_BUFFER, m_ebo, m_glIndexBufferCapacity);

            m_meshes.push_back({ mesh, vertexBufferId, indexBufferId});
        }
    }

    void BatchMesh::RecreateGlBuffer(const GLuint glTarget, const uint32_t capacity, GLuint& glBuffer)
    {
        if (glBuffer != GL_NONE)
        {
            glDeleteBuffers(1, &glBuffer);
        }

        GetRS()->BindVertexArray(m_vao);
        glGenBuffers(1, &glBuffer);
        
        GetRS()->BindBuffer(glTarget, glBuffer);
        glBufferData(glTarget, static_cast<GLsizeiptr>(capacity), nullptr, GL_STATIC_DRAW);

        if (glTarget == GL_ARRAY_BUFFER)
        {
            uint32_t offset = 0;
            for (auto& attr : VERTEX_ATTR_DEFINES)
            {
                glEnableVertexAttribArray(static_cast<uint32_t>(attr.attr));
                glVertexAttribPointer(
                    static_cast<uint32_t>(attr.attr),
                    static_cast<GLint>(attr.strideF),
                    GL_FLOAT,
                    GL_FALSE,
                    static_cast<GLsizei>(MAX_VERTEX_ATTR_STRIDE_F * sizeof(float)),
                    reinterpret_cast<const void*>(offset));
                offset += attr.strideF * sizeof(float);
            }
        }
    }

    ManagedBlockId BatchMesh::AddDataToLocal(const void* src, const uint32_t size, ManagedBuffer* dstBuffer)
    {
        auto blockId = dstBuffer->Alloc(size);
        auto dst = dstBuffer->GetWriteDst(blockId);
        memcpy(dst, src, size);
        return blockId;
    }

    void BatchMesh::SyncLocalDataToGl(
        ManagedBuffer* localBuffer,
        const ManagedBlockId blockId,
        const GLuint glTarget,
        GLuint& glBufferId,
        uint32_t& glBufferCapacity)
    {
        uint32_t blockOffset, blockSize;
        if (glBufferCapacity != localBuffer->Capacity())
        {
            glBufferCapacity = localBuffer->Capacity();
            RecreateGlBuffer(glTarget, glBufferCapacity, glBufferId);
            blockOffset = 0;
            blockSize = localBuffer->Size();
        }
        else
        {
            localBuffer->GetBlockInfo(blockId, blockOffset, blockSize);
        }

        GetRS()->BindVertexArray(GL_NONE);
        GetRS()->BindBuffer(glTarget, glBufferId);
        glBufferSubData(glTarget, blockOffset, blockSize, localBuffer->Data() + blockOffset);
    }

    const std::vector<float>& BatchMesh::GetCompleteVertexData(Mesh* mesh)
    {
        auto vertexCount = mesh->vertexCount;
        auto& rawVertexData = mesh->GetVertexData();
        auto rawVertexStrideF = mesh->vertexDataStrideB / sizeof(float);
        
        static std::vector<float> vertexData;
        vertexData.resize(MAX_VERTEX_ATTR_STRIDE_F * vertexCount);

        uint32_t curOffsetF = 0;
        for (auto& [attr, attrStrideF, name] : VERTEX_ATTR_DEFINES)
        {
            auto aa = name;
            auto& meshVertexAttrInfo = mesh->vertexAttribInfo.at(attr);
            if (meshVertexAttrInfo.enabled)
            {
                auto rawAttrOffsetF = meshVertexAttrInfo.offsetB / sizeof(float);
                for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
                {
                    for (uint32_t j = 0; j < attrStrideF; ++j)
                    {
                        auto dstIndex = vertexIndex * MAX_VERTEX_ATTR_STRIDE_F + curOffsetF + j;
                        auto srcIndex = vertexIndex * rawVertexStrideF + rawAttrOffsetF + j;
                        vertexData[dstIndex] = rawVertexData[srcIndex];
                    }
                }
            }

            curOffsetF += attrStrideF;
        }

        return vertexData;
    }
}
