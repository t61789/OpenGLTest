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
        friend class MeshCacheMgr;
        
        struct Cache
        {
            Bounds bounds = {};
            uint32_t vertexCount = 0;
            vec<float> vertexData = {};
            vec<uint32_t> indices = {};

            template <class Archive>
            void serialize(Archive& ar, unsigned int version);
        };
        
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
        
        static Cache CreateCacheFromAsset(crstr assetPath);
        static sp<Mesh> CreateAssetFromCache(Cache&& cache);
        
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

        static sp<Mesh> LoadFromFileImp(crstr modelPath);
        static up<Assimp::Importer> ImportFile(crstr modelPath);
        static void GetMeshLoadConfig(crstr modelPath, float& initScale, bool& flipWindingOrder);
        static sp<Mesh> CreateMesh(
            umap<VertexAttr, VertexAttrInfo>&& vertexAttribInfo,
            vec<float>&& vertexData,
            vec<uint32_t>&& indices,
            cr<Bounds> bounds,
            uint32_t vertexCount);
        static void CalcVertexAttrOffset(std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo);
        
        static crvec<float> GetFullVertexData(const Mesh* mesh);
    };

    template <class Archive>
    void Mesh::Cache::serialize(Archive& ar, unsigned int version)
    {
        ar & bounds;
        ar & vertexCount;
        ar & vertexData;
        ar & indices;
    }
}
