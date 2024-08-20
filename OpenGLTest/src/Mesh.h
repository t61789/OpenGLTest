#pragma once
#include <string>
#include <glad/glad.h>

class Mesh
{
public:
    GLuint VAO, VBO, EBO;
    size_t vertexCount;

    Mesh();
    Mesh(const float* position, const float* normal, const float* texcoord, const float* color, const unsigned int* indices, size_t vertexCount);
    ~Mesh();
    
    static Mesh* LoadFromFile(std::string modelPath);

    void Use() const;
};
