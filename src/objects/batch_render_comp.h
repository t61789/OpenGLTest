#pragma once

#include "bounds.h"
#include "comp.h"
#include "culling_system.h"
#include "event.h"
#include "render/batch_matrix.h"

namespace op
{
    class Mesh;
    class Material;
    class Comp;
    
    class BatchRenderComp final : public Comp
    {
    public:
        crsp<Mesh> GetMesh() const { return m_mesh;}
        crsp<Material> GetMaterial() const { return m_material;}
        
        void OnEnable() override;
        void OnDisable() override;
        void UpdateTransform();
        bool HasONS();
        Bounds GetWorldBounds();

        bool GetInView() { return m_cullingBufferAccessor.IsEnable() ? m_cullingBufferAccessor.GetVisible() : false; }

        void LoadFromJson(cr<nlohmann::json> objJson) override;

    private:
        sp<Mesh> m_mesh = nullptr;
        sp<Material> m_material = nullptr;
        EventHandler m_onTransformDirtyHandler = 0;
        bool m_transformDirty = true;
        BatchMatrix::Elem m_submitBuffer;
        Bounds m_worldBounds;
        CullingBufferAccessor m_cullingBufferAccessor = {};

        void OnTransformDirty();
        void UpdatePerObjectBuffer();
    };
}
