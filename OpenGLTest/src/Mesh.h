#pragma once
#include <string>
#include <glad/glad.h>

#include "ConstVars.h"

class Mesh
{
public:
    GLuint VAO, VBO, EBO;
    size_t vertexCount;
    bool vertexAttribEnabled[VERTEX_ATTRIB_NUM];
    int vertexDataStride;
    int vertexDataOffset[VERTEX_ATTRIB_NUM];

    Mesh();
    Mesh(const float* position, const float* normal, const float* texcoord, const float* color, const unsigned int* indices, size_t vertexCount);
    ~Mesh();
    
    static Mesh* LoadFromFile(std::string modelPath);

    void Use() const;
};
