#pragma once

#include "Comp.h"
#include "glm/glm.hpp"

#include "Object.h"

class LightComp : public Comp
{
public:
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1;
    int lightType = 0; // 0 parallel, 1 point

    void LoadFromJson(const nlohmann::json& objJson) override;
};
