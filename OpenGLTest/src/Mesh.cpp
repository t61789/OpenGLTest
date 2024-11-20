#include "Mesh.h"

#include <iostream>

#include "ResourceMgr.h"
#include "Utils.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

void copyDataTo(
    const float* src,
    float* dest,
    const unsigned int offset,
    const unsigned int floatCount,
    const size_t vertexCount,
    const size_t vertexDataStride)
{
    if(src == nullptr)
    {
        return;
    }

    for(unsigned int i = 0; i < floatCount * vertexCount; i++)
    {
        dest[offset + i / floatCount * vertexDataStride + i % floatCount] = src[i];
    }
}

Mesh::Mesh()
{
    m_id = ResourceMgr::AddPtr(this);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);

    ResourceMgr::RemovePtr(m_id);
}

RESOURCE_ID Mesh::CreateMesh(
    const float* position,
    const float* normal,
    const float* uv0,
    const float* color,
    const unsigned int* indices,
    const size_t vertexCount)
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
    auto vertexDataStride = 0;
    bool vertexAttribEnabled[vertexAttribNum];
    int vertexAttribOffset[vertexAttribNum] = {};
    for (size_t i = 0; i < vertexAttribNum; ++i)
    {
        vertexAttribEnabled[i] = vertexAttribSource[i] != nullptr;
        if(vertexAttribEnabled[i])
        {
            vertexAttribOffset[i] = vertexDataStride;
            vertexDataStride += VERTEX_ATTRIB_FLOAT_COUNT[i];
        }
    }
    size_t vertexDataSumSize = vertexDataStride * vertexCount;

    // 将多个独立存储属性的数组合并成一个属性交叉的顶点数据数组
    auto data = new float[vertexDataSumSize];
    for (size_t i = 0; i < vertexAttribNum; ++i)
    {
        if(vertexAttribEnabled[i])
        {
            copyDataTo(
                vertexAttribSource[i],
                data,
                vertexAttribOffset[i],
                VERTEX_ATTRIB_FLOAT_COUNT[i],
                vertexCount,
                vertexDataStride);
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexCount * sizeof(int)), indices, GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    delete[] data;

    auto result = new Mesh();
    result->m_vao = VAO;
    result->m_vbo = VBO;
    result->m_ebo = EBO;
    result->m_vertexCount = vertexCount;
    std::memcpy(&result->m_vertexAttribEnabled, &vertexAttribEnabled, sizeof(vertexAttribEnabled));
    std::memcpy(&result->m_vertexAttribOffset, &vertexAttribOffset, sizeof(vertexAttribOffset));
    return result->m_id;
}

RESOURCE_ID Mesh::LoadFromFile(const std::string& modelPath)
{
    if(ResourceMgr::IsResourceRegistered(modelPath))
    {
        return ResourceMgr::GetRegisteredResource(modelPath);
    }
    
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(Utils::GetRealAssetPath(modelPath).c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error(std::string("ERROR>> Load model failed: ") + importer.GetErrorString());
    }

    auto mesh = scene->mMeshes[0];

    // Load positionOS
    std::vector<float> positionOSContainer;
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        positionOSContainer.push_back(mesh->mVertices[i].x);
        positionOSContainer.push_back(mesh->mVertices[i].y);
        positionOSContainer.push_back(mesh->mVertices[i].z);
    }
    auto positionOSData = positionOSContainer.data();

    // Load normalOS
    float* normalOSData = nullptr;
    std::vector<float> normalsContainer;
    if(mesh->mNormals)
    {
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
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
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            uv0Container.push_back(mesh->mTextureCoords[i]->x);
            uv0Container.push_back(mesh->mTextureCoords[i]->y);
        }
        uv0Data = uv0Container.data();
    }

    // Load indices
    std::vector<unsigned int> indicesContainer;
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        auto face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            indicesContainer.push_back(face.mIndices[j]);
        }
    }
    auto indicesData = indicesContainer.data();
    auto indicesCount = indicesContainer.size();

    auto result = CreateMesh(
        positionOSData,
        normalOSData,
        uv0Data,
        nullptr,
        indicesData,
        indicesCount);
    ResourceMgr::RegisterResource(modelPath, result);
    return result;
}

void Mesh::Use() const
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
}


