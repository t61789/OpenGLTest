#pragma once
#include <vec3.hpp>

class Object
{
public:
    glm::vec3 position;
    glm::vec3 rotation;

    Object();
    Object(glm::vec3 position, glm::vec3 rotation);

    virtual ~Object();

    virtual void Update();
};
