#pragma once
#include <string>
#include <glad/glad.h>

#include "ConstVars.h"
#include "SharedObject.h"
#include "Utils.h"

class Mesh : public SharedObject
{
public:
    std::string name = "Unnamed Mesh";
    
    GLuint vao, vbo, ebo;

    Bounds bounds;
    size_t vertexCount;
    size_t indicesCount;
    int vertexDataFloatNum;
    bool vertexAttribEnabled[VERTEX_ATTRIB_NUM];
    int vertexAttribOffset[VERTEX_ATTRIB_NUM];

    ~Mesh() override;
    
    void use() const;
    
    static Mesh* LoadFromFile(const std::string& modelPath);
    static Mesh* CreateMesh(
        const Bounds& bounds,
        const float* position,
        const float* normal,
        const float* uv0,
        const float* color,
        const unsigned int* indices,
        size_t vertexCount,
        size_t indicesCount,
        const std::string& name = "Unnamed Mesh"
    );
};
