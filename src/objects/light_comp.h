﻿#pragma once

#include "comp.h"
#include "glm/glm.hpp"

#include "object.h"

namespace op
{
    class LightComp : public Comp
    {
    public:
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1;
        int lightType = 0; // 0 parallel, 1 point
        float radius = 10;

        void LoadFromJson(const nlohmann::json& objJson) override;
    };
}
