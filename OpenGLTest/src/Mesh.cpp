#include "Mesh.h"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

Mesh::Mesh() = default;

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

Mesh::Mesh(
    const float* position,
    const float* normal,
    const float* texcoord,
    const float* color,
    const unsigned int* indices,
    const size_t vertexCount):
vertexCount(vertexCount)
{
    const float* vertexAttribSource[] = {
        position,
        normal,
        texcoord,
        color
    };

    auto vertexAttribNum = sizeof(VERTEX_ATTRIB_FLOAT_COUNT) / sizeof(int);
    vertexDataStride = 0;
    for (size_t i = 0; i < vertexAttribNum; ++i)
    {
        vertexAttribEnabled[i] = vertexAttribSource[i];
        if(vertexAttribSource[i])
        {
            vertexDataStride += VERTEX_ATTRIB_FLOAT_COUNT[i];
        }
    }
    size_t dataLength = vertexDataStride * vertexCount;

    auto data = new float[dataLength];
    int curOffset = 0;
    for (size_t i = 0; i < vertexAttribNum; ++i)
    {
        if(vertexAttribSource[i])
        {
            copyDataTo(
                vertexAttribSource[i],
                data,
                curOffset,
                VERTEX_ATTRIB_FLOAT_COUNT[i],
                vertexCount,
                vertexDataStride);
            vertexDataOffset[i] = curOffset;
            curOffset += VERTEX_ATTRIB_FLOAT_COUNT[i];
        }
    }
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(dataLength * sizeof(float)), data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexCount * sizeof(int)), indices, GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    delete[] data;
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Mesh* Mesh::LoadFromFile(std::string modelPath)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(modelPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error(std::string("ERROR>> Load model failed: ") + importer.GetErrorString());
    }

    auto mesh = scene->mMeshes[0];
    std::vector<float> positionsContainer;
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        positionsContainer.push_back(mesh->mVertices[i].x);
        positionsContainer.push_back(mesh->mVertices[i].y);
        positionsContainer.push_back(mesh->mVertices[i].z);
    }

    float* normals = nullptr;
    std::vector<float> normalsContainer;
    if(mesh->mNormals)
    {
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            normalsContainer.push_back(mesh->mNormals[i].x);
            normalsContainer.push_back(mesh->mNormals[i].y);
            normalsContainer.push_back(mesh->mNormals[i].z);
        }
        normals = normalsContainer.data();
    }

    float* texcoords = nullptr;
    std::vector<float> texcoordsContainer;
    if(mesh->HasTextureCoords(0))
    {
        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            texcoordsContainer.push_back(mesh->mTextureCoords[i]->x);
            texcoordsContainer.push_back(mesh->mTextureCoords[i]->y);
        }
        texcoords = texcoordsContainer.data();
    }

    std::vector<unsigned int> indicesContainer;
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        auto face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            indicesContainer.push_back(face.mIndices[j]);
        }
    }

    return new Mesh(
        positionsContainer.data(),
        normals,
        texcoords,
        nullptr,
        indicesContainer.data(),
        indicesContainer.size());
}

void Mesh::Use() const
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
}


