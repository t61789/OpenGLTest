#pragma once
#include "Mesh.h"
#include "Object.h"
#include "Shader.h"

class Material;

class Entity : public Object
{
public:

    Shader* shader;
    Mesh* mesh;
    Material* mat;

    Entity(Shader* shader, Mesh* mesh, Material* mat);
};
