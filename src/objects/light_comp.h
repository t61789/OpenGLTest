#pragma once

#include "math/math.h"
#include "comp.h"

#include "object.h"

namespace op
{
    class LightComp : public Comp
    {
    public:
        Vec3 color = Vec3(1.0f);
        float intensity = 1;
        int lightType = 0; // 0 parallel, 1 point
        float radius = 10;

        Vec3 GetColor() const;

        void LoadFromJson(const nlohmann::json& objJson) override;
    };
}
