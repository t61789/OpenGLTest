#include "Object.h"

Object::Object()
{
    position = glm::vec3(0, 0, 0);
    rotation = glm::vec3(0, 0, 0);
}

Object::Object(glm::vec3 position, glm::vec3 rotation):
position(position),
rotation(rotation)
{
}

Object::~Object()
{
    
}

void Object::Update()
{
    // pass
}
