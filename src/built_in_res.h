#pragma once
#include "shader.h"
#include "utils.h"
#include "material.h"

#define BUILT_IN_RES (BuiltInRes::Ins())

namespace op
{
    class Mesh;

    class BuiltInRes : public Singleton<BuiltInRes>
    {
    public:
        BuiltInRes();
        ~BuiltInRes();

        Mesh* quadMesh = nullptr;
        Mesh* sphereMesh = nullptr;

        Material* blitMatNew = nullptr;

        Texture* errorTex = nullptr;
        Texture* whiteTex = nullptr;
        Texture* blackTex = nullptr;
        Texture* missTex = nullptr;

        std::vector<Material*> predefinedMaterials;

    private:
        std::vector<Shader*> m_packedShaders;
        
        void LoadPackedShaders();
    };
}
