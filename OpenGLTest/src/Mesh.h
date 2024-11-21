#pragma once
#include <string>
#include <glad/glad.h>

#include "ConstVars.h"
#include "ResourceMgr.h"
#include "Utils.h"

class Mesh : public ResourceBase
{
public:
    GLuint vao, vbo, ebo;

    Bounds bounds;
    size_t vertexCount;
    size_t indicesCount;
    int vertexDataFloatNum;
    bool vertexAttribEnabled[VERTEX_ATTRIB_NUM];
    int vertexAttribOffset[VERTEX_ATTRIB_NUM];
    
    void use() const;
    
    static RESOURCE_ID LoadFromFile(const std::string& modelPath);
    static RESOURCE_ID CreateMesh(
        const Bounds& bounds,
        const float* position,
        const float* normal,
        const float* uv0,
        const float* color,
        const unsigned int* indices,
        size_t vertexCount,
        size_t indicesCount
    );

private:

    ~Mesh() override;
};
