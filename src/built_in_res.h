#pragma once
#include "utils.h"

namespace op
{
    class Material;
    class Mesh;
    class Image;

    class BuiltInRes : public Singleton<BuiltInRes>
    {
    public:
        BuiltInRes();
        ~BuiltInRes();
    
        Mesh* quadMesh = nullptr;
        Mesh* sphereMesh = nullptr;

        Material* blitMat = nullptr;

        Image* errorTex = nullptr;
    };
}
