#include "mesh.h"

#include "utils.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <fstream>
#include <mutex>
#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "common/asset_cache.h"
#include "render/gl/gl_buffer.h"
#include "render/gl/gl_vertex_array.h"

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
        
        ZoneScoped;

        auto result = AssetCache::GetFromCache<Mesh, Cache>(modelPath);

        GetGR()->RegisterResource(modelPath, result);
        result->m_path = modelPath;
        
        log_info("Load mesh: %s", modelPath.c_str());
        
        return result;
    }

    sp<Mesh> Mesh::LoadFromFileImp(crstr modelPath)
    {
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
            std::move(vertexAttribInfo),
            std::move(vertexData),
            std::move(indices),
            bounds,
            verticesCount);

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
        umap<VertexAttr, VertexAttrInfo>&& vertexAttribInfo,
        vec<float>&& vertexData,
        vec<uint32_t>&& indices,
        cr<Bounds> bounds,
        const uint32_t vertexCount)
    {
        auto vertexDataStrideB = static_cast<int>(vertexData.size() / vertexCount * sizeof(float));

        auto vao = msp<GlVertexArray>();
        auto vbo = msp<GlBuffer>(GL_ARRAY_BUFFER);
        auto ebo = msp<GlBuffer>(GL_ELEMENT_ARRAY_BUFFER);

        vbo->SetData(GL_STATIC_DRAW, vertexData.size() * sizeof(float), vertexData.data());
        ebo->SetData(GL_STATIC_DRAW, indices.size() * sizeof(uint32_t), indices.data());

        {
            std::lock_guard usingVao(*vao);
            
            vao->BindVbo(vbo);
            vao->BindEbo(ebo);

            for (auto& [attr, attrInfo] : vertexAttribInfo)
            {
                auto index = find_index_if(VERTEX_ATTR_DEFINES, [a=attr](cr<VertexAttrDefine> x)
                {
                    return x.attr == a;
                });
                assert(index.has_value());
                
                auto& attrDefine = VERTEX_ATTR_DEFINES[index.value()];
                if (attrInfo.enabled)
                {
                    vao->SetAttrEnable(index.value(), true);
                    vao->SetAttr(attrDefine.attr, vertexDataStrideB, attrInfo.offsetB);
                }
                else
                {
                    vao->SetAttrEnable(index.value(), false);
                }
            }
        }
        
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
    
    crvec<float> Mesh::GetFullVertexData(const Mesh* mesh)
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

    Mesh::Cache Mesh::CreateCacheFromAsset(crstr assetPath)
    {
        auto mesh = LoadFromFileImp(assetPath);

        Cache fullMesh;
        fullMesh.bounds = mesh->GetBounds();
        fullMesh.vertexCount = mesh->GetVertexCount();
        fullMesh.vertexData = GetFullVertexData(mesh.get());
        fullMesh.indices = mesh->GetIndexData();

        return fullMesh;
    }

    sp<Mesh> Mesh::CreateAssetFromCache(Cache&& cache)
    {
        auto c = std::move(cache);
        
        auto mesh = CreateMesh(
            CreateFullVertexAttribInfo(),
            std::move(c.vertexData),
            std::move(c.indices),
            c.bounds,
            c.vertexCount);

        return mesh;
    }
}
