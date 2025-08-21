#pragma once

#include "nlohmann/json.hpp"

#include "bounds.h"
#include "comp.h"
#include "event.h"
#include "material.h"
#include "render/per_object_buffer.h"

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
        
        void UpdateTransform();

        const Bounds& GetWorldBounds();
        uint32_t GetObjectIndex() const { return m_perObjectBufferIndex; }

    private:
        EventHandler m_onTransformDirtyHandler = 0;
        bool m_transformDirty = true;
        Bounds m_worldBounds;
        uint32_t m_perObjectBufferIndex = ~0u;
        PerObjectStruct m_submitBuffer;

        void OnTransformDirty();

        void UpdateWorldBounds();
        void UpdatePerObjectBuffer();
    };
}
