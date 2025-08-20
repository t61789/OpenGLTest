#pragma once
#include <string>
#include <assimp/Importer.hpp>

#include "bounds.h"
#include "const_vars.h"
#include "shared_object.h"
#include "utils.h"

namespace op
{
    class Mesh : public SharedObject
    {
    public:
        class VertexAttrInfo
        {
        public:
            bool enabled = false;
            uintptr_t offset = 0;
        };
        
        std::string name = "Unnamed Mesh";
        
        GLuint vao, vbo, ebo;

        Bounds bounds;
        uint32_t vertexCount;
        int indicesCount;
        int vertexDataFloatNum;
        bool vertexAttribEnabled[VERTEX_ATTRIB_NUM];
        int vertexAttribOffset[VERTEX_ATTRIB_NUM];

        std::unordered_map<VertexAttr, VertexAttrInfo> vertexAttribInfo;
        uint32_t vertexDataStride;

        ~Mesh() override;
        
        static Mesh* LoadFromFile(const std::string& modelPath);
        

    private:
        static std::unique_ptr<Assimp::Importer> ImportFile(const std::string& modelPath);
        static void GetMeshLoadConfig(const std::string& modelPath, float& initScale, bool& flipWindingOrder);
        static Mesh* CreateMesh(
            const std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo,
            const std::vector<float>& vertexData,
            const std::vector<uint32_t>& indices,
            const Bounds& bounds,
            uint32_t vertexCount,
            const std::string& name = "Unnamed Mesh");
        static void CalcVertexAttrOffset(std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo);
    };
}
