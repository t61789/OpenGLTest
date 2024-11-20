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

private:

    Mesh();
    ~Mesh();

    static RESOURCE_ID CreateMesh(
        const float* position,
        const float* normal,
        const float* uv0,
        const float* color,
        const unsigned int* indices,
        const size_t vertexCount
    );
};
