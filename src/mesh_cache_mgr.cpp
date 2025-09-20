#include "mesh_cache_mgr.h"

#include <fstream>
#include <boost/archive/binary_oarchive.hpp>

#include "game_resource.h"
#include "imgui_impl_opengl3_loader.h"
#include "mesh.h"

namespace
{
    using namespace op;
    
    std::unordered_map<VertexAttr, Mesh::VertexAttrInfo> CreateFullVertexAttribInfo()
    {
        umap<VertexAttr, Mesh::VertexAttrInfo> vertexAttribInfo;
        for (auto& attrInfo : VERTEX_ATTR_DEFINES)
        {
            Mesh::VertexAttrInfo meshVertexAttrInfo;
            meshVertexAttrInfo.enabled = true;
            meshVertexAttrInfo.offsetB = attrInfo.offsetF * sizeof(float);
            vertexAttribInfo.emplace(attrInfo.attr, meshVertexAttrInfo);
        }

        return vertexAttribInfo;
    }
}

namespace op
{
    sp<Mesh> MeshCacheMgr::GetMeshFromCache(cr<std::string> objFilePath)
    {
        auto fullMeshPath = GetFullMeshPath(objFilePath);
        auto fullMeshMetaPath = GetFullMeshMetaPath(objFilePath);

        auto absFullMeshPath = Utils::GetAbsolutePath(fullMeshPath);
        auto absFullMeshMetaPath = Utils::GetAbsolutePath(fullMeshMetaPath);

        if (!std::filesystem::exists(absFullMeshPath) || !std::filesystem::exists(absFullMeshMetaPath))
        {
            DoCache(objFilePath);
        }

        FullMesh fullMesh;
        FullMeshMeta fullMeshMeta;
        LoadCache(objFilePath, fullMesh, fullMeshMeta);

        auto mesh = Mesh::CreateMesh(
            CreateFullVertexAttribInfo(),
            std::move(fullMesh.vertexData),
            std::move(fullMesh.indices),
            fullMesh.bounds,
            fullMesh.vertexCount);

        return mesh;
    }

    void MeshCacheMgr::LoadCache(cr<std::string> objFilePath, FullMesh& fullMesh, FullMeshMeta& fullMeshMeta)
    {
        auto fullMeshPath = GetFullMeshPath(objFilePath);
        auto fullMeshMetaPath = GetFullMeshMetaPath(objFilePath);
        
        Utils::BinaryDeserialize(fullMeshMeta, fullMeshMetaPath);
        size_t objFileHash = Utils::GetFileHash(objFilePath);
        if (fullMeshMeta.objFileHash != objFileHash)
        {
            DoCache(objFilePath);
        }

        Utils::BinaryDeserialize(fullMesh, fullMeshPath);
        Utils::BinaryDeserialize(fullMeshMeta, fullMeshMetaPath);
    }

    void MeshCacheMgr::DoCache(cr<std::string> objPath)
    {
        auto mesh = Mesh::LoadFromFileImp(objPath);

        FullMesh fullMesh;
        fullMesh.bounds = mesh->GetBounds();
        fullMesh.vertexCount = mesh->GetVertexCount();
        fullMesh.vertexData = GetFullVertexData(mesh.get());
        fullMesh.indices = mesh->GetIndexData();
        
        FullMeshMeta fullMeshMeta;
        fullMeshMeta.objFileHash = GetObjFileHash(objPath);
        
        Utils::BinarySerialize(fullMesh, GetFullMeshPath(objPath));
        Utils::BinarySerialize(fullMeshMeta, GetFullMeshMetaPath(objPath));

        log_info("Mesh cached success: %s", objPath.c_str());
    }

    size_t MeshCacheMgr::GetObjFileHash(cr<std::string> path)
    {
        return Utils::GetFileHash(path);
    }

    crvec<float> MeshCacheMgr::GetFullVertexData(const Mesh* mesh)
    {
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

    std::string MeshCacheMgr::GetFullMeshPath(cr<std::string> path)
    {
        return "cache/mesh/" + path;
    }

    std::string MeshCacheMgr::GetFullMeshMetaPath(cr<std::string> path)
    {
        return "cache/mesh/" + path + ".meta";
    }
}
