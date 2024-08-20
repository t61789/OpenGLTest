#pragma once
#include "Object.h"

class Camera : public Object
{
public:
    Camera(glm::vec3 position, glm::vec3 rotation);
    void Update() override;
private:

    glm::vec3 _targetPosition;
    glm::vec3 _targetRotation;
};
