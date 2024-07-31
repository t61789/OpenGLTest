#include "Mesh.h"

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
    const float* texcoord,
    const float* color,
    const unsigned int* indices,
    const size_t vertexCount):
vertexCount(vertexCount)
{
    constexpr size_t vertexDataStride = 3 + 2 + 3;
    size_t dataLength = vertexDataStride * vertexCount;

    auto data = new float[dataLength];

    copyDataTo(position, data, 0, 3, vertexCount, vertexDataStride);
    copyDataTo(texcoord, data, 3, 2, vertexCount, vertexDataStride);
    copyDataTo(color, data, 5, 3, vertexCount, vertexDataStride);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(dataLength * sizeof(float)), data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexCount * sizeof(int)), indices, GL_STATIC_DRAW);

    if(position)
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * vertexDataStride, (void*)0);
        glEnableVertexAttribArray(0);
    }
    if(texcoord)
    {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * vertexDataStride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    if(color)
    {
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * vertexDataStride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    delete[] data;
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::use() const
{
    glBindVertexArray(VAO);
}


