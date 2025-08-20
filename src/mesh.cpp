#include "mesh.h"

#include "shared_object.h"
#include "utils.h"
#include "assimp/Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#include <fstream>

#include "render_context.h"

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
            if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, VertexAttr::POSITION_OS)->stride > 3)
            {
                vertexData.push_back(1.0f);
            }

            // Load normalOS
            if (vertexAttribInfo[VertexAttr::NORMAL_OS].enabled)
            {
                vertexData.push_back(mesh->mNormals[i].x);
                vertexData.push_back(mesh->mNormals[i].y);
                vertexData.push_back(mesh->mNormals[i].z);
                if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, VertexAttr::NORMAL_OS)->stride > 3)
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
                if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, VertexAttr::TANGENT_OS)->stride > 3)
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
                if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, uvAttr[j])->stride > 3)
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
        
        log_info("成功载入Mesh: %s", modelPath.c_str());
        return result;
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
            THROW_ERROR("Load model failed: %s", importer->GetErrorString())
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
        auto vertexDataStride = static_cast<int>(vertexData.size() / vertexCount * sizeof(float));
        
        // 将顶点数据传入OpenGL中
        GLuint vao, vbo, ebo;
        
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        auto renderState = RenderState::Ins();
        // renderState->SetVertexArray(vao);
        glBindVertexArray(vao);
        

        renderState->BindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexData.size() * sizeof(float)), vertexData.data(), GL_STATIC_DRAW);

        renderState->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(int)), indices.data(), GL_STATIC_DRAW);

        for (auto& [attr, attrInfo] : vertexAttribInfo)
        {
            auto slot = find_index(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, attr);
            auto& attrDefine = VERTEX_ATTR_DEFINES[slot];
            if (attrInfo.enabled)
            {
                glEnableVertexAttribArray(slot);
                glVertexAttribPointer(
                    slot,
                    attrDefine.stride,
                    GL_FLOAT,
                    GL_FALSE,
                    vertexDataStride,
                    reinterpret_cast<const void*>(attrInfo.offset));
            }
            else
            {
                glDisableVertexAttribArray(slot);
            }
        }
        
        // // renderState->SetVertexArray(GL_NONE);
        // glBindVertexArray(GL_NONE);
        // // renderState->BindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        // glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

        GL_CHECK_ERROR(传输Mesh数据)
        
        auto result = new Mesh();
        result->bounds = bounds;
        result->vao = vao;
        result->vbo = vbo;
        result->ebo = ebo;
        result->vertexCount = vertexCount;
        result->vertexDataStride = vertexDataStride;
        result->indicesCount = static_cast<int>(indices.size());
        result->name = name;
        result->vertexAttribInfo = vertexAttribInfo;
        return result;
    }

    void Mesh::CalcVertexAttrOffset(std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo)
    {
        size_t curOffset = 0;
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
