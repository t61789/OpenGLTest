#pragma once
#include <assimp/Importer.hpp>

#include "bounds.h"
#include "i_resource.h"
#include "utils.h"

namespace op
{
    class GlVertexArray;
    class GlBuffer;
    
    class Mesh final : public IResource
    {
        friend sp<Mesh> std::make_shared<Mesh>();
        
    public:
        struct VertexAttrInfo
        {
            bool enabled = false;
            uintptr_t offsetB = 0;
        };

        Mesh() = default;
        
        void Use();

        cr<StringHandle> GetPath() override { return m_path;}
        cr<Bounds> GetBounds() const { return m_bounds;}
        uint32_t GetVertexDataStrideB() const { return m_vertexDataStrideB;}
        crvec<float> GetVertexData() const { return m_vertexData;}
        crvec<uint32_t> GetIndexData() const { return m_indexData;}
        crumap<VertexAttr, VertexAttrInfo> GetVertexAttribInfo() const { return m_vertexAttribInfo;}
        
        uint32_t GetVertexCount() const { return static_cast<uint32_t>(GetVertexData().size() * sizeof(float) / GetVertexDataStrideB());}
        uint32_t GetIndicesCount() const { return static_cast<uint32_t>(GetIndexData().size());}

        static sp<Mesh> LoadFromFile(crstr modelPath);

    private:
        sp<GlVertexArray> m_vao;
        sp<GlBuffer> m_vbo;
        sp<GlBuffer> m_ebo;

        Bounds m_bounds;
        StringHandle m_path;
        
        uint32_t m_vertexDataStrideB;
        vec<float> m_vertexData;
        vec<uint32_t> m_indexData;
        umap<VertexAttr, VertexAttrInfo> m_vertexAttribInfo;
        
        static up<Assimp::Importer> ImportFile(crstr modelPath);
        static void GetMeshLoadConfig(crstr modelPath, float& initScale, bool& flipWindingOrder);
        static sp<Mesh> CreateMesh(
            std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo,
            std::vector<float>& vertexData,
            std::vector<uint32_t>& indices,
            const Bounds& bounds,
            uint32_t vertexCount);
        static void CalcVertexAttrOffset(std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo);
    };
}
