#pragma once

#include <json.hpp>

#include "Bounds.h"
#include "Comp.h"

namespace op
{
    class Mesh;
    class Material;

    class RenderComp : public Comp
    {
    public:
        Mesh* mesh = nullptr;
        Material* material = nullptr;
    
        Bounds bounds;

        ~RenderComp() override;
    
        void LoadFromJson(const nlohmann::json& objJson) override;
    };
}
