#pragma once
#include <boost/serialization/vector.hpp>

#include "bounds.h"
#include "math/vec.h"

namespace op
{
    class Mesh;

    struct FullMesh
    {
        Bounds bounds = {};
        uint32_t vertexCount = 0;
        vec<float> vertexData = {};
        vec<uint32_t> indices = {};

        template <class Archive>
        void serialize(Archive& ar, unsigned int version);
    };

    struct FullMeshMeta
    {
        size_t objFileHash = 0;

        template <class Archive>
        void serialize(Archive& ar, unsigned int version);
    };
    
    class MeshCacheMgr
    {
    public:
        static sp<Mesh> GetMeshFromCache(cr<std::string> objFilePath);

    private:
        static void LoadCache(cr<std::string> objFilePath, FullMesh& fullMesh, FullMeshMeta& fullMeshMeta);
        static void DoCache(cr<std::string> objPath);

        static size_t GetObjFileHash(cr<std::string> path);
        static crvec<float> GetFullVertexData(const Mesh* mesh);
        static std::string GetFullMeshPath(cr<std::string> path);
        static std::string GetFullMeshMetaPath(cr<std::string> path);
    };

    template <class Archive>
    void FullMesh::serialize(Archive& ar, unsigned int version)
    {
        ar & bounds;
        ar & vertexCount;
        ar & vertexData;
        ar & indices;
    }

    template <class Archive>
    void FullMeshMeta::serialize(Archive& ar, unsigned int version)
    {
        ar & objFileHash;
    }
}
