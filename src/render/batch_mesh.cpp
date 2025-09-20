#include "batch_mesh.h"

#include <mutex>

#include "mesh.h"
#include "common/elem_accessor_var.h"
#include "gl/gl_state.h"
#include "render/gl/gl_vertex_array.h"
#include "render/gl/gl_submit_buffer.h"

namespace op
{
    BatchMesh::BatchMesh()
    {
        m_vao = msp<GlVertexArray>();
        m_vbo = msp<GlSubmitBuffer>(
            static_cast<uint32_t>(GL_ARRAY_BUFFER),
            static_cast<uint32_t>(GL_STATIC_DRAW),
            static_cast<uint32_t>(500000 * MAX_VERTEX_ATTR_STRIDE_F * sizeof(float)));

        m_ebo = msp<GlSubmitBuffer>(
            static_cast<uint32_t>(GL_ELEMENT_ARRAY_BUFFER),
            static_cast<uint32_t>(GL_STATIC_DRAW),
            static_cast<uint32_t>(100000 * sizeof(uint32_t)));

        m_vboAccessor = mup<ElemAccessorVar>(m_vbo);
        m_eboAccessor = mup<ElemAccessorVar>(m_ebo);

        {
            std::lock_guard usingVao(*m_vao);
            
            m_vao->BindVbo(m_vbo->GetGlBuffer());
            m_vao->BindEbo(m_ebo->GetGlBuffer());
            ConfigVaoPointer(m_vao);
        }
    }

    void BatchMesh::Use()
    {
        assert(m_delayAddMeshes.empty());
        
        m_vao->Use();
    }

    void BatchMesh::StopUse()
    {
        m_vao->StopUse();
    }

    void BatchMesh::RegisterMesh(crsp<Mesh> mesh)
    {
        m_delayAddMeshes.push_back(mesh);
    }

    void BatchMesh::GetMeshInfo(Mesh* mesh, uint32_t& vertexOffsetB, uint32_t& vertexSizeB, uint32_t& indexOffsetB, uint32_t& indexSizeB)
    {
        RegisterMeshActually();

        auto meshInfo = find_if(m_meshes, [mesh](cr<MeshInfo> x){ return x.mesh.get() == mesh;});
        if (!meshInfo)
        {
            THROW_ERROR("Unregistered mesh!")
        }
        
        m_vboAccessor->GetInfo(meshInfo->vertexBufferId, vertexOffsetB, vertexSizeB);
        m_eboAccessor->GetInfo(meshInfo->indexBufferId, indexOffsetB, indexSizeB);
    }

    void BatchMesh::RegisterMeshActually()
    {
        if (m_delayAddMeshes.empty())
        {
            return;
        }
        
        vecwp<Mesh> needAddMeshes;
        size_t addVertexDataSizeB = 0;
        size_t addIndexDataSizeB = 0;
        for (auto& meshPtr : m_delayAddMeshes)
        {
            if (!meshPtr.expired() &&
                !exists_if(m_meshes, [meshPtr](cr<MeshInfo> x){ return x.mesh == meshPtr.lock();}) &&
                !exists_if(needAddMeshes, [meshPtr](crwp<Mesh> x){ return x.lock() == meshPtr.lock();}))
            {
                auto mesh = meshPtr.lock().get();
                needAddMeshes.push_back(meshPtr);
                addVertexDataSizeB += MAX_VERTEX_ATTR_STRIDE_F * mesh->GetVertexCount() * sizeof(float);
                addIndexDataSizeB += mesh->GetIndicesCount() * sizeof(uint32_t);
            }
        }
        m_delayAddMeshes.clear();

        m_vboAccessor->Reserve(m_vboAccessor->Size() + addVertexDataSizeB);
        m_eboAccessor->Reserve(m_eboAccessor->Size() + addIndexDataSizeB);

        for (const auto& meshPtr : needAddMeshes)
        {
            auto mesh = meshPtr.lock().get();
            auto& vertexData = GetFullVertexData(meshPtr);
            auto& indexData = mesh->GetIndexData();

            auto vertexBufferId = m_vboAccessor->Alloc(vertexData.size() * sizeof(float));
            m_vboAccessor->Set(vertexBufferId, vertexData.data());
            
            auto indexBufferId = m_eboAccessor->Alloc(indexData.size() * sizeof(uint32_t));
            m_eboAccessor->Set(indexBufferId, indexData.data());

            m_meshes.push_back({ meshPtr.lock(), vertexBufferId, indexBufferId});
        }

        m_vbo->Submit();
        m_ebo->Submit();
    }

    crvec<float> BatchMesh::GetFullVertexData(crwp<Mesh> meshPtr)
    {
        auto mesh = meshPtr.lock().get();
        auto vertexCount = mesh->GetVertexCount();
        auto& rawVertexData = mesh->GetVertexData();
        auto rawVertexStrideF = mesh->GetVertexDataStrideB() / sizeof(float);
        
        static std::vector<float> vertexData;
        vertexData.resize(MAX_VERTEX_ATTR_STRIDE_F * vertexCount);

        uint32_t curOffsetF = 0;
        for (auto& attrInfo : VERTEX_ATTR_DEFINES)
        {
            auto aa = attrInfo.name;
            auto& meshVertexAttrInfo = mesh->GetVertexAttribInfo().at(attrInfo.attr);
            if (meshVertexAttrInfo.enabled)
            {
                auto rawAttrOffsetF = meshVertexAttrInfo.offsetB / sizeof(float);
                for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
                {
                    for (uint32_t j = 0; j < attrInfo.strideF; ++j)
                    {
                        auto dstIndex = vertexIndex * MAX_VERTEX_ATTR_STRIDE_F + curOffsetF + j;
                        auto srcIndex = vertexIndex * rawVertexStrideF + rawAttrOffsetF + j;
                        vertexData[dstIndex] = rawVertexData[srcIndex];
                    }
                }
            }

            curOffsetF += attrInfo.strideF;
        }

        return vertexData;
    }
    
    void BatchMesh::ConfigVaoPointer(crsp<GlVertexArray> vao)
    {
        for (auto& attr : VERTEX_ATTR_DEFINES)
        {
            vao->SetAttrEnable(static_cast<uint32_t>(attr.attr), true);
            vao->SetAttr(
                attr.attr,
                MAX_VERTEX_ATTR_STRIDE_F * sizeof(float),
                attr.offsetF * sizeof(float));
        }
    }
}
