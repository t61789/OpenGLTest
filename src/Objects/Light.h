#pragma once

#include "glm/glm.hpp"

#include "Object.h"

class Light : public Object
{
public:
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1;
    int lightType = 0; // 0 parallel, 1 point

private:
    void LoadFromJson(const nlohmann::json& objJson) override;
};
