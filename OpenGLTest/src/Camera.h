#pragma once
#include "Object.h"
#include "GameFramework.h"

class Camera : public Object
{
public:
    Camera(glm::vec3 position, glm::vec3 rotation);
    void Update() override;
private:
};
