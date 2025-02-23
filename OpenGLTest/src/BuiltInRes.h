#pragma once
#include "Material.h"
#include "Mesh.h"

class BuiltInRes
{
public:
    Mesh* quadMesh = nullptr;
    Mesh* sphereMesh = nullptr;

    Material* blitMat = nullptr;

    static BuiltInRes* GetInstance();

private:
    BuiltInRes();
};
