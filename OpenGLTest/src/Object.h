#pragma once
#include <vec3.hpp>
#include "glm.hpp"

class Object
{
public:
    glm::vec3 position;
    glm::vec3 rotation;

    Object();
    Object(glm::vec3 position, glm::vec3 rotation);
    virtual ~Object();

    glm::mat4 GetLocalToWorld() const;
    virtual void Update();
};
