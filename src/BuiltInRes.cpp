#include "BuiltInRes.h"

#include "Mesh.h"
#include "Material.h"
#include "Image.h"

BuiltInRes* BuiltInRes::m_instance = nullptr;

BuiltInRes* BuiltInRes::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new BuiltInRes();
    }
    return m_instance;
}

void BuiltInRes::ReleaseInstance()
{
    if (!m_instance)
    {
        return;
    }

    delete m_instance;
    m_instance = nullptr;
}

BuiltInRes::BuiltInRes()
{
    quadMesh = Mesh::LoadFromFile("meshes/quad.obj");
    INCREF(quadMesh);
    sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
    INCREF(sphereMesh);

    blitMat = Material::CreateEmptyMaterial("shaders/blit.glsl");
    INCREF(blitMat);

    auto desc = ImageDescriptor::GetDefault();
    errorTex = Image::LoadFromFile("built_in/texture/error.png", desc);
    INCREF(errorTex);
}

BuiltInRes::~BuiltInRes()
{
    DECREF(quadMesh);
    DECREF(sphereMesh);
    DECREF(blitMat);
    DECREF(errorTex);
}


