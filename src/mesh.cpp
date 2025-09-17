#include "mesh.h"

#include "shared_object.h"
#include "utils.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <fstream>

#include "game_resource.h"
#include "render/gl/gl_buffer.h"
#include "render/gl/gl_state.h"
#include "render/gl/gl_vertex_array.h"

namespace op
{
    void Mesh::Use()
    {
        m_vao->Use();
    }

    sp<Mesh> Mesh::LoadFromFile(crstr modelPath)
    {
        {
            if (auto mesh = GetGR()->GetResource<Mesh>(modelPath))
            {
                return mesh;
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
            if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, VertexAttr::POSITION_OS)->strideF > 3)
            {
                vertexData.push_back(1.0f);
            }

            // Load normalOS
            if (vertexAttribInfo[VertexAttr::NORMAL_OS].enabled)
            {
                vertexData.push_back(mesh->mNormals[i].x);
                vertexData.push_back(mesh->mNormals[i].y);
                vertexData.push_back(mesh->mNormals[i].z);
                if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, VertexAttr::NORMAL_OS)->strideF > 3)
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
                if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, VertexAttr::TANGENT_OS)->strideF > 3)
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
                if (find(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, uvAttr[j])->strideF > 3)
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
            verticesCount);

        GetGR()->RegisterResource(result->GetPath(), result);
        result->m_path = modelPath;
        
        log_info("成功载入Mesh: %s", modelPath.c_str());
        
        return result;
    }

    up<Assimp::Importer> Mesh::ImportFile(crstr modelPath)
    {
        float initScale;
        bool flipWindingOrder;
        GetMeshLoadConfig(modelPath, initScale, flipWindingOrder);
        
        auto importer = mup<Assimp::Importer>();
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
            THROW_ERRORF("Load model failed: %s", importer->GetErrorString())
        }

        return importer;
    }
    
    sp<Mesh> Mesh::CreateMesh(
        std::unordered_map<VertexAttr, VertexAttrInfo>& vertexAttribInfo,
        std::vector<float>& vertexData,
        std::vector<uint32_t>& indices,
        const Bounds& bounds,
        const uint32_t vertexCount)
    {
        auto vertexDataStrideB = static_cast<int>(vertexData.size() / vertexCount * sizeof(float));

        auto vao = msp<GlVertexArray>();
        auto vbo = msp<GlBuffer>(GL_ARRAY_BUFFER);
        auto ebo = msp<GlBuffer>(GL_ELEMENT_ARRAY_BUFFER);

        vbo->SetData(GL_STATIC_DRAW, vertexData.size() * sizeof(float), vertexData.data());
        ebo->SetData(GL_STATIC_DRAW, indices.size() * sizeof(uint32_t), indices.data());

        vao->StartSetting();
        vao->BindVbo(vbo);
        vao->BindEbo(ebo);

        for (auto& [attr, attrInfo] : vertexAttribInfo)
        {
            auto index = find_index(VERTEX_ATTR_DEFINES, &VertexAttrDefine::attr, attr);
            auto& attrDefine = VERTEX_ATTR_DEFINES[index];
            if (attrInfo.enabled)
            {
                vao->SetAttrEnable(index, true);
                vao->SetAttr(attrDefine.attr, vertexDataStrideB, attrInfo.offsetB);
            }
            else
            {
                vao->SetAttrEnable(index, false);
            }
        }

        vao->EndSetting();
        
        auto result = std::make_shared<Mesh>();
        result->m_bounds = bounds;
        result->m_vao = std::move(vao);
        result->m_vbo = std::move(vbo);
        result->m_ebo = std::move(ebo);
        result->m_vertexDataStrideB = vertexDataStrideB;
        result->m_vertexAttribInfo = std::move(vertexAttribInfo);
        result->m_vertexData = std::move(vertexData);
        result->m_indexData = std::move(indices);
        
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

            vertexAttribInfo[attr].offsetB = curOffset * sizeof(float);
            curOffset += VERTEX_ATTR_STRIDE[attr];
        }
    }

    void Mesh::GetMeshLoadConfig(crstr modelPath, float& initScale, bool& flipWindingOrder)
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
