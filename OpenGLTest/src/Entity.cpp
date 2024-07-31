#include "Entity.h"

Entity::Entity(Shader* shader, Mesh* mesh):
shader(shader),
mesh(mesh)
{
}

void Entity::draw()
{
    shader->use();
}
