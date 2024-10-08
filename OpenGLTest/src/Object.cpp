﻿#include "Object.h"

#include "gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/euler_angles.hpp"

Object::Object()
{
    position = glm::vec3(0, 0, 0);
    scale = glm::vec3(1, 1, 1);
    rotation = glm::vec3(0, 0, 0);
}

Object::Object(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation):
position(position),
scale(scale),
rotation(rotation)
{
}

Object::~Object() = default;

glm::mat4 Object::GetLocalToWorld() const
{
    auto objectMatrix = glm::mat4(1);
    objectMatrix = translate(objectMatrix, position);
    objectMatrix = objectMatrix * glm::eulerAngleYXZ(
        glm::radians(rotation.y),
        glm::radians(rotation.x),
        glm::radians(rotation.z));
    objectMatrix = glm::scale(objectMatrix, scale);
    return objectMatrix;
}

void Object::Update()
{
    // pass
}
