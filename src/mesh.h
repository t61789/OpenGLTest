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
            uintptr_t offsetB = 0;
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
        uint32_t vertexDataStrideB;

        ~Mesh() override;

        const std::vector<float>& GetVertexData() const { return m_vertexData;}
        const std::vector<uint32_t>& GetIndexData() const { return m_indexData;}
        
        static Mesh* LoadFromFile(const std::string& modelPath);

    private:
        std::vector<float> m_vertexData;
        std::vector<uint32_t> m_indexData;
        
        static std::unique_ptr<Assimp::Importer> ImportFile(const std::string& modelPath);
        static void GetMeshLoadConfig(const std::string& modelPath, float& initScale, bool& flipWindingOrder);
        static Mesh* CreateMesh(
            std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo,
            std::vector<float>& vertexData,
            std::vector<uint32_t>& indices,
            const Bounds& bounds,
            uint32_t vertexCount,
            const std::string& name = "Unnamed Mesh");
        static void CalcVertexAttrOffset(std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo);
    };
}
