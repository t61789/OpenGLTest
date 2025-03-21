﻿#include "built_in_res.h"

#include "mesh.h"
#include "material.h"
#include "image.h"

namespace op
{
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
}
