#include "mesh.h"

#include "shared_object.h"
#include "utils.h"
#include "assimp/Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include <fstream>

namespace op
{
    static void CopyDataTo(
        const float* src,
        float* dest,
        const unsigned int offset,
        const unsigned int attribFloatNum,
        const int vertexCount,
        const int vertexDataFloatNum)
    {
        if(src == nullptr)
        {
            return;
        }

        for(unsigned int i = 0; i < attribFloatNum * vertexCount; i++)
        {
            auto vertexIndex = i / attribFloatNum;
            auto destIndex = offset + vertexIndex * vertexDataFloatNum + i % attribFloatNum;
            auto srcIndex = i;
            dest[destIndex] = src[srcIndex];
        }
    }

    Mesh* Mesh::CreateMesh(
        const Bounds& bounds,
        const float* position,
        const float* normal,
        const float* uv0,
        const float* color,
        const unsigned int* indices,
        const uint32_t verticesCount,
        const uint32_t indicesCount,
        const std::string& name)
    {
        const float* vertexAttribSource[] = {
            position,
            normal,
            uv0,
            color
        };

        // 根据是否提供了对应属性数组的指针来计算顶点数据的步长
        // 并记录每一个属性在一个步长中的偏移值
        constexpr auto vertexAttribNum = std::size(vertexAttribSource);
        auto vertexDataFloatNum = 0;
        bool vertexAttribEnabled[vertexAttribNum];
        int vertexAttribOffset[vertexAttribNum] = {};
        for (int i = 0; i < vertexAttribNum; ++i)
        {
            vertexAttribEnabled[i] = vertexAttribSource[i] != nullptr;
            if(vertexAttribEnabled[i])
            {
                vertexAttribOffset[i] = vertexDataFloatNum;
                vertexDataFloatNum += VERTEX_ATTRIB_FLOAT_COUNT[i];
            }
        }
        int vertexDataSumSize = vertexDataFloatNum * verticesCount;

        // 将多个独立存储属性的数组合并成一个属性交叉的顶点数据数组
        auto data = new float[vertexDataSumSize];
        for (int i = 0; i < vertexAttribNum; ++i)
        {
            if(vertexAttribEnabled[i])
            {
                CopyDataTo(
                    vertexAttribSource[i],
                    data,
                    vertexAttribOffset[i],
                    VERTEX_ATTRIB_FLOAT_COUNT[i],
                    verticesCount,
                    vertexDataFloatNum);
            }
        }

        // 将顶点数据传入OpenGL中
        GLuint VAO, VBO, EBO;
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexDataSumSize * sizeof(float)), data, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indicesCount * sizeof(int)), indices, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        delete[] data;

        auto result = new Mesh();
        result->bounds = bounds;
        result->vao = VAO;
        result->vbo = VBO;
        result->ebo = EBO;
        result->vertexDataFloatNum = vertexDataFloatNum;
        result->vertexCount = verticesCount;
        result->indicesCount = indicesCount;
        result->name = name;
        std::memcpy(&result->vertexAttribEnabled, &vertexAttribEnabled, sizeof(vertexAttribEnabled));
        std::memcpy(&result->vertexAttribOffset, &vertexAttribOffset, sizeof(vertexAttribOffset));
        return result;
    }

    Mesh::~Mesh()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    Mesh* Mesh::LoadFromFile(const std::string& modelPath)
    {
        {
            SharedObject* result;
            if(TryGetResource(modelPath, result))
            {
                return dynamic_cast<Mesh*>(result);
            }
        }
        
        float init_scale;
        bool flip_winding_order;
        GetMeshLoadConfig(modelPath, init_scale, flip_winding_order);
        
        Assimp::Importer importer;
        importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, init_scale);

        unsigned int pFlags = 
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_GlobalScale;

        if (flip_winding_order)
        {
            pFlags |= aiProcess_FlipWindingOrder;
        }
        
        const aiScene *scene = importer.ReadFile(Utils::GetAbsolutePath(modelPath).c_str(), pFlags);
        
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error(std::string("ERROR>> Load model failed: ") + importer.GetErrorString());
        }

        auto mesh = scene->mMeshes[0];
        auto verticesCount = mesh->mNumVertices;

        // Load positionOS
        auto boundsMin = Vec3(std::numeric_limits<float>::max());
        auto boundsMax = Vec3(std::numeric_limits<float>::min());
        std::vector<float> positionOSContainer;
        for (uint32_t i = 0; i < verticesCount; ++i)
        {
            auto x = mesh->mVertices[i].x;
            auto y = mesh->mVertices[i].y;
            auto z = mesh->mVertices[i].z;
            
            boundsMin.x = std::min(boundsMin.x, x);
            boundsMin.y = std::min(boundsMin.y, y);
            boundsMin.z = std::min(boundsMin.z, z);
            boundsMax.x = std::max(boundsMax.x, x);
            boundsMax.y = std::max(boundsMax.y, y);
            boundsMax.z = std::max(boundsMax.z, z);
            
            positionOSContainer.push_back(x);
            positionOSContainer.push_back(y);
            positionOSContainer.push_back(z);
        }
        auto positionOSData = positionOSContainer.data();

        // Load normalOS
        float* normalOSData = nullptr;
        std::vector<float> normalsContainer;
        if(mesh->mNormals)
        {
            for (uint32_t i = 0; i < verticesCount; ++i)
            {
                normalsContainer.push_back(mesh->mNormals[i].x);
                normalsContainer.push_back(mesh->mNormals[i].y);
                normalsContainer.push_back(mesh->mNormals[i].z);
            }
            normalOSData = normalsContainer.data();
        }

        // Load UV0
        float* uv0Data = nullptr;
        std::vector<float> uv0Container;
        if(mesh->HasTextureCoords(0))
        {
            for (uint32_t i = 0; i < verticesCount; ++i)
            {
                uv0Container.push_back(mesh->mTextureCoords[0][i].x);
                uv0Container.push_back(mesh->mTextureCoords[0][i].y);
            }
            uv0Data = uv0Container.data();
        }

        // Load indices
        std::vector<unsigned int> indicesContainer;
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            auto face = mesh->mFaces[i];
            // std::string a;
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
            {
                indicesContainer.push_back(face.mIndices[j]);
                // a += " " + std::to_string(face.mIndices[j]);
            }
            // Utils::LogInfo(a);
        }
        auto indicesData = indicesContainer.data();
        auto indicesCount = static_cast<uint32_t>(indicesContainer.size());

        auto bounds = Bounds((boundsMax + boundsMin) * 0.5f, (boundsMax - boundsMin) * 0.5f);
        bounds.extents = max(bounds.extents, Vec3(0.01f));

        auto result = CreateMesh(
            bounds,
            positionOSData,
            normalOSData,
            uv0Data,
            nullptr,
            indicesData,
            verticesCount,
            indicesCount,
            modelPath);
        RegisterResource(modelPath, result);
        auto msg = "成功载入Mesh " + modelPath + "\n";
        msg += "\t顶点数量 " + std::to_string(verticesCount) + "\n";
        msg += "\t三角形数量 " + std::to_string(indicesCount / 3) + "\n";
        msg += "\t包围盒 c:" + bounds.center.ToString() + " e:" + bounds.extents.ToString();
        Utils::LogInfo(msg);
        return result;
    }
    
    Mesh* Mesh::LoadFromFile0(const std::string& modelPath)
    {
        {
            SharedObject* result;
            if(TryGetResource(modelPath, result))
            {
                return dynamic_cast<Mesh*>(result);
            }
        }

        auto importer = ImportFile(modelPath);
        auto scene = importer->GetScene();

        auto mesh = scene->mMeshes[0];
        auto verticesCount = mesh->mNumVertices;
        
        std::unordered_map<VertexAttr, VertexAttrInfo> vertexAttribInfo;
        vertexAttribInfo[VertexAttr::POSITION_OS] = {true};
        vertexAttribInfo[VertexAttr::NORMAL_OS] = {mesh->mNormals != nullptr};
        vertexAttribInfo[VertexAttr::TANGENT_OS] = {mesh->mTangents != nullptr};
        vertexAttribInfo[VertexAttr::UV0] = {mesh->HasTextureCoords(0)};
        vertexAttribInfo[VertexAttr::UV1] = {mesh->HasTextureCoords(1)};
        CalcVertexAttrOffset(vertexAttribInfo);

        auto boundsMin = Vec3(std::numeric_limits<float>::max());
        auto boundsMax = Vec3(std::numeric_limits<float>::min());

        // 遍历每个顶点，得到交错的顶点数据
        std::vector<float> vertexData;
        for (uint32_t i = 0; i < verticesCount; ++i)
        {
            // Load positionOS
            auto positionOS = Vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z);
            boundsMin = min(boundsMin, positionOS);
            boundsMax = max(boundsMax, positionOS);
            vertexData.push_back(positionOS.x);
            vertexData.push_back(positionOS.y);
            vertexData.push_back(positionOS.z);
            if (VERTEX_ATTR_STRIDE[VertexAttr::POSITION_OS] > 3)
            {
                vertexData.push_back(1.0f);
            }

            // Load normalOS
            if (vertexAttribInfo[VertexAttr::NORMAL_OS].enabled)
            {
                vertexData.push_back(mesh->mNormals[i].x);
                vertexData.push_back(mesh->mNormals[i].y);
                vertexData.push_back(mesh->mNormals[i].z);
                if (VERTEX_ATTR_STRIDE[VertexAttr::NORMAL_OS] > 3)
                {
                    vertexData.push_back(0.0f);
                }
            }

            // Load tangentOS
            if (vertexAttribInfo[VertexAttr::TANGENT_OS].enabled)
            {
                vertexData.push_back(mesh->mTangents[i].x);
                vertexData.push_back(mesh->mTangents[i].y);
                vertexData.push_back(mesh->mTangents[i].z);
                if (VERTEX_ATTR_STRIDE[VertexAttr::TANGENT_OS] > 3)
                {
                    vertexData.push_back(1.0f);
                }
            }

            // Load uv
            VertexAttr uvAttr[] = {VertexAttr::UV0, VertexAttr::UV1};
            for (size_t j = 0; j < 2; ++j)
            {
                if (!vertexAttribInfo[uvAttr[j]].enabled)
                {
                    continue;
                }
                
                vertexData.push_back(mesh->mTextureCoords[j][i].x);
                vertexData.push_back(mesh->mTextureCoords[j][i].y);
                if (VERTEX_ATTR_STRIDE[uvAttr[j]] > 2)
                {
                    vertexData.push_back(0.0f);
                }
            }
        }

        // Load indices
        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            auto face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        auto bounds = Bounds((boundsMax + boundsMin) * 0.5f, (boundsMax - boundsMin) * 0.5f);
        bounds.extents = max(bounds.extents, Vec3(0.01f));

        auto result = CreateMesh(
            vertexAttribInfo,
            vertexData,
            indices,
            bounds,
            verticesCount,
            modelPath);
        RegisterResource(modelPath, result);
        
        Utils::LogInfo("成功载入Mesh: %s", modelPath.c_str());
        return result;
    }

    void Mesh::Use() const
    {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        Utils::CheckGlError("启用Mesh");
    }

    std::unique_ptr<Assimp::Importer> Mesh::ImportFile(const std::string& modelPath)
    {
        float initScale;
        bool flipWindingOrder;
        GetMeshLoadConfig(modelPath, initScale, flipWindingOrder);
        
        auto importer = std::make_unique<Assimp::Importer>();
        importer->SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, initScale);

        unsigned int pFlags = 
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_GlobalScale;

        if (flipWindingOrder)
        {
            pFlags |= aiProcess_FlipWindingOrder;
        }
        
        const aiScene *scene = importer->ReadFile(Utils::GetAbsolutePath(modelPath).c_str(), pFlags);
        
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error(std::string("ERROR>> Load model failed: ") + importer->GetErrorString());
        }

        return importer;
    }
    
    Mesh* Mesh::CreateMesh(
        const std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo,
        const std::vector<float>& vertexData,
        const std::vector<uint32_t>& indices,
        const Bounds& bounds,
        const uint32_t vertexCount,
        const std::string& name)
    {
        // 将顶点数据传入OpenGL中
        GLuint vao, vbo, ebo;
        
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexData.size() * sizeof(float)), vertexData.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(int)), indices.data(), GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        auto result = new Mesh();
        result->bounds = bounds;
        result->vao = vao;
        result->vbo = vbo;
        result->ebo = ebo;
        result->vertexCount = vertexCount;
        result->vertexDataStride = static_cast<int>(vertexData.size() / vertexCount * sizeof(float));
        result->indicesCount = static_cast<int>(indices.size());
        result->name = name;
        result->vertexAttribInfo = vertexAttribInfo;
        return result;
    }

    void Mesh::CalcVertexAttrOffset(std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo)
    {
        uint32_t curOffset = 0;
        for (uint8_t i = 0; i < static_cast<uint8_t>(VertexAttr::COUNT); ++i)
        {
            auto attr = static_cast<VertexAttr>(i);
            if (!vertexAttribInfo[attr].enabled)
            {
                continue;
            }

            vertexAttribInfo[attr].offset = curOffset * sizeof(float);
            curOffset += VERTEX_ATTR_STRIDE[attr];
        }
    }

    void Mesh::GetMeshLoadConfig(const std::string& modelPath, float& initScale, bool& flipWindingOrder)
    {
        auto config = Utils::GetResourceMeta(modelPath);

        initScale = 1.0f;
        if (config.contains("init_scale"))
        {
            initScale = config.at("init_scale").get<float>();
        }

        flipWindingOrder = true;
        if (config.contains("flip_winding_order"))
        {
            flipWindingOrder = config.at("flip_winding_order").get<bool>();
        }
    }
}
