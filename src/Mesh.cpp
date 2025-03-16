#include "Mesh.h"

#include "SharedObject.h"
#include "Utils.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <fstream>

void copyDataTo(
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
            copyDataTo(
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
    
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(Utils::GetAbsolutePath(modelPath).c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw std::runtime_error(std::string("ERROR>> Load model failed: ") + importer.GetErrorString());
    }

    auto mesh = scene->mMeshes[0];
    auto verticesCount = mesh->mNumVertices;

    // Load positionOS
    auto boundsMin = glm::vec3(std::numeric_limits<float>::max());
    auto boundsMax = glm::vec3(std::numeric_limits<float>::min());
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
    bounds.extents = max(bounds.extents, glm::vec3(0.01f));

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
    msg += "\t包围盒 c:" + Utils::ToString(bounds.center) + " e:" + Utils::ToString(bounds.extents);
    Utils::LogInfo(msg);
    return result;
}

void Mesh::Use() const
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}
