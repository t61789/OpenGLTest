#pragma once

#include "nlohmann/json.hpp"

#include "bounds.h"
#include "comp.h"
#include "event.h"

namespace op
{
    class Mesh;
    class Material;

    class RenderComp : public Comp
    {
    public:
        Mesh* mesh = nullptr;
        Material* material = nullptr;
    
        void Awake() override;

        void Start() override;

        void OnDestroy() override;
    
        void LoadFromJson(const nlohmann::json& objJson) override;

        const Bounds& GetWorldBounds();

    private:
        EventHandler m_onTransformDirtyHandler = 0;
        bool m_worldBoundsDirty = true;
        Bounds m_worldBounds;

        void OnTransformDirty();

        void UpdateWorldBounds();
    };
}
