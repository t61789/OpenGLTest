#pragma once
#include <string>
#include <glad/glad.h>

#include "ConstVars.h"
#include "ResourceMgr.h"

class Mesh
{
public:
    RESOURCE_ID m_id;
    GLuint m_vao, m_vbo, m_ebo;
    
    size_t m_vertexCount;
    int vertexDataStride;
    bool m_vertexAttribEnabled[VERTEX_ATTRIB_NUM];
    int m_vertexAttribOffset[VERTEX_ATTRIB_NUM];
    
    void Use() const;
    
    static RESOURCE_ID LoadFromFile(const std::string& modelPath);
    static RESOURCE_ID CreateMesh(
        const float* position,
        const float* normal,
        const float* uv0,
        const float* color,
        const unsigned int* indices,
        size_t vertexCount
    );

private:

    Mesh();
    ~Mesh();
};
