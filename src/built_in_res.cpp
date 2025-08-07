#include "built_in_res.h"

#include <fstream>
#include <sstream>

#include "mesh.h"
#include "material.h"
#include "image.h"

namespace op
{
    BuiltInRes::BuiltInRes()
    {
        LoadPackedShaders();
        
        quadMesh = Mesh::LoadFromFile("meshes/quad.obj");
        INCREF(quadMesh);
        sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
        INCREF(sphereMesh);

        blitMat = Material::CreateEmptyMaterial("shaders/blit.glsl");
        INCREF(blitMat);

        auto desc = ImageDescriptor::GetDefault();
        errorTex = Image::LoadFromFile("built_in/texture/error.png", desc);
        INCREF(errorTex);

        testMesh = Mesh::LoadFromFile0("built_in/meshes/quad.obj");
        INCREF(testMesh);
        testShader = Shader::LoadFromFile("shaders/test.shader");
        INCREF(testShader);
        testMaterial = Material::CreateEmptyMaterial(testShader, "TestMaterial");
        testMaterial->cullMode = CullMode::None;
        INCREF(testMaterial);
    }

    BuiltInRes::~BuiltInRes()
    {
        for (auto shader : m_packedShaders)
        {
            DECREF(shader);
        }
        
        DECREF(quadMesh);
        DECREF(sphereMesh);
        DECREF(blitMat);
        DECREF(errorTex);
        DECREF(testMesh);
        DECREF(testShader);
        DECREF(testMaterial);
    }

    void BuiltInRes::LoadPackedShaders()
    {
        auto shaderPackPath = "shaders.pack";
        
        auto s = std::ifstream(Utils::GetAbsolutePath(shaderPackPath));
        nlohmann::json json;
        s >> json;
        s.close();

        for (auto& [path, shaderStr] : json.items())
        {
            auto vertStr = Utils::Binary8To32(Utils::Base64ToBinary(shaderStr["vert"].get<std::string>()));
            auto fragStr = Utils::Binary8To32(Utils::Base64ToBinary(shaderStr["frag"].get<std::string>()));

            auto shader = Shader::LoadFromSpvFile(vertStr, fragStr, path);
            m_packedShaders.push_back(shader);
            INCREF(shader);
        }
    }
}
