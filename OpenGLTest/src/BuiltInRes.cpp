#include "BuiltInRes.h"

BuiltInRes* BuiltInRes::GetInstance()
{
    static BuiltInRes* instance = nullptr;
    if (instance == nullptr)
    {
        instance = new BuiltInRes();
    }
    return instance;
}

BuiltInRes::BuiltInRes()
{
    quadMesh = Mesh::LoadFromFile("meshes/quad.obj");
    quadMesh->IncRef();
    sphereMesh = Mesh::LoadFromFile("meshes/sphere.obj");
    sphereMesh->IncRef();

    blitMat = Material::CreateEmptyMaterial("shaders/blit.glsl");
    blitMat->IncRef();
}


