#include "built_in_res.h"

#include "mesh.h"

#include "image.h"
#include "material.h"
#include "shader.h"

namespace op
{
    BuiltInRes::BuiltInRes()
    {
        LoadPackedShaders();
        
        quadMesh = Mesh::LoadFromFile("built_in/meshes/quad.obj");
        sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");

        blitMatNew = Material::LoadFromFile("built_in/materials/blit_mat.json");

        errorTex = Image::LoadFromFile("built_in/texture/error.png");
        whiteTex = Image::LoadFromFile("built_in/texture/white.png");
        blackTex = Image::LoadFromFile("built_in/texture/black.png");
        missTex = Image::LoadFromFile("built_in/texture/miss.png");
    }

    void BuiltInRes::LoadPackedShaders()
    {
        auto shaderPackPath = "shaders.pack";
        auto json = Utils::LoadJson(shaderPackPath);

        for (auto& [path, shaderStr] : json.items())
        {
            const auto& shader = Shader::LoadFromSpvBase64(
                shaderStr["vert"].get<std::string>(),
                shaderStr["frag"].get<std::string>(),
                path);
            
            m_packedShaders.push_back(shader);
        }
    }
}
