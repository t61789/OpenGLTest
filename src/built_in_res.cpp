#include "built_in_res.h"

#include <fstream>
#include <sstream>

#include "mesh.h"

#include "image.h"

namespace op
{
    BuiltInRes::BuiltInRes()
    {
        for (const auto& nameId : PREDEFINED_MATERIALS)
        {
            auto mat = new Material();
            INCREF(mat);

            GetGR()->SubmitPredefinedMaterial(nameId, mat);
            predefinedMaterials.push_back(mat);
        }

        LoadPackedShaders();
        
        quadMesh = Mesh::LoadFromFile("built_in/meshes/quad.obj");
        INCREF(quadMesh);
        sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
        INCREF(sphereMesh);

        blitMatNew = Material::LoadFromFile("built_in/materials/blit_mat.json");
        INCREF(blitMatNew);

        auto desc = ImageDescriptor::GetDefault();
        errorTex = Image::LoadFromFile("built_in/texture/error.png", desc);
        INCREF(errorTex);
        whiteTex = Image::LoadFromFile("built_in/texture/white.png", desc);
        INCREF(whiteTex);
        blackTex = Image::LoadFromFile("built_in/texture/black.png", desc);
        INCREF(blackTex);
        missTex = Image::LoadFromFile("built_in/texture/miss.png", desc);
        INCREF(missTex);

        testShader = Shader::LoadFromFile("shaders/test.shader");
        INCREF(testShader);

        testMaterialNew = new Material();
        INCREF(testMaterialNew);
        testMaterialNew->BindShader(testShader);
    }

    BuiltInRes::~BuiltInRes()
    {
        for (auto mat : predefinedMaterials)
        {
            DECREF(mat);
        }
        
        for (auto shader : m_packedShaders)
        {
            DECREF(shader);
        }
        
        DECREF(quadMesh);
        DECREF(sphereMesh);
        DECREF(blitMatNew);
        DECREF(errorTex);
        DECREF(whiteTex);
        DECREF(blackTex);
        DECREF(testShader);
        DECREF(testMaterialNew);
    }

    void BuiltInRes::LoadPackedShaders()
    {
        auto shaderPackPath = "shaders.pack";
        auto json = Utils::LoadJson(shaderPackPath);

        for (auto& [path, shaderStr] : json.items())
        {
            auto shader = Shader::LoadFromSpvBase64(
                shaderStr["vert"].get<std::string>(),
                shaderStr["frag"].get<std::string>(),
                path);
            
            m_packedShaders.push_back(shader);
            INCREF(shader);
        }
    }
}
