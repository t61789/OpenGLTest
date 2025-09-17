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

        auto desc = ImageDescriptor::GetDefault();
        errorTex = Image::LoadFromFile("built_in/texture/error.png", desc);
        whiteTex = Image::LoadFromFile("built_in/texture/white.png", desc);
        blackTex = Image::LoadFromFile("built_in/texture/black.png", desc);
        missTex = Image::LoadFromFile("built_in/texture/miss.png", desc);
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
