#pragma once
#include "Mesh.h"
#include "Object.h"
#include "Shader.h"

class Entity : Object
{
public:

    Shader* shader;
    Mesh* mesh;

    Entity(Shader* shader, Mesh* mesh);

    void draw();
};
