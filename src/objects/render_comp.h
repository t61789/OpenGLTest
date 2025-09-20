#pragma once

#include "nlohmann/json.hpp"

#include "bounds.h"
#include "comp.h"
#include "event.h"

namespace op
{
    class Mesh;
    class Material;

    class RenderComp final : public Comp
    {
    public:
        void Awake() override;
        void Start() override;
        void OnDestroy() override;
        void UpdateTransform();
        cr<Bounds> GetWorldBounds();
        
        bool HasOddNegativeScale() const;
        sp<Mesh> GetMesh() const { return m_mesh;}
        sp<Material> GetMaterial() const { return m_material;}
        uint32_t GetObjectIndex() const { return m_perObjectBufferIndex;}

        void LoadFromJson(const nlohmann::json& objJson) override;

    private:
        sp<Mesh> m_mesh = nullptr;
        sp<Material> m_material = nullptr;
    
        EventHandler m_onTransformDirtyHandler = 0;
        bool m_transformDirty = true;
        Bounds m_worldBounds;
        uint32_t m_perObjectBufferIndex = ~0u;

        void OnTransformDirty();

        void UpdateWorldBounds();
        void UpdatePerObjectBuffer();
    };
}
