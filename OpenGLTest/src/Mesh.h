#pragma once
#include <glad/glad.h>

class Mesh
{
public:
    GLuint VAO, VBO, EBO;
    size_t vertexCount;

    Mesh();
    Mesh(const float* position, const float* texcoord, const float* color, const unsigned int* indices, size_t vertexCount);
    ~Mesh();

    void Use() const;
};
