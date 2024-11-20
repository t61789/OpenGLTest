#pragma once
#include <string>
#include <glad/glad.h>

#include "ConstVars.h"
#include "ResourceMgr.h"
#include "Utils.h"

class Mesh : public ResourceBase
{
public:
    GLuint m_vao, m_vbo, m_ebo;

    Bounds m_bounds;
    size_t m_vertexCount;
    size_t m_indicesCount;
    int m_vertexDataFloatNum;
    bool m_vertexAttribEnabled[VERTEX_ATTRIB_NUM];
    int m_vertexAttribOffset[VERTEX_ATTRIB_NUM];
    
    void Use() const;
    
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
