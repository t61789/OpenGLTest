﻿#pragma once
#include <string>

#include "bounds.h"
#include "const_vars.h"
#include "shared_object.h"
#include "utils.h"

namespace op
{
    class Mesh : public SharedObject
    {
    public:
        std::string name = "Unnamed Mesh";
        
        GLuint vao, vbo, ebo;

        Bounds bounds;
        int vertexCount;
        int indicesCount;
        int vertexDataFloatNum;
        bool vertexAttribEnabled[VERTEX_ATTRIB_NUM];
        int vertexAttribOffset[VERTEX_ATTRIB_NUM];

        ~Mesh() override;
        
        void Use() const;
        
        static Mesh* LoadFromFile(const std::string& modelPath);
        static Mesh* CreateMesh(
            const Bounds& bounds,
            const float* position,
            const float* normal,
            const float* uv0,
            const float* color,
            const unsigned int* indices,
            uint32_t verticesCount,
            uint32_t indicesCount,
            const std::string& name = "Unnamed Mesh"
        );
    };
}
