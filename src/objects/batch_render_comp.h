#pragma once

#include "comp.h"
#include "event.h"
#include "math/matrix4x4.h"
#include "render/batch_matrix.h"

namespace op
{
    class Mesh;
    class Material;
    class Comp;
    
    class BatchRenderComp : public Comp
    {
    public:
        ~BatchRenderComp() override;
        
        Mesh* GetMesh() const { return m_mesh;}
        Material* GetMaterial() const { return m_material;}
        void OnEnable() override;
        void OnDisable() override;
        void UpdateTransform();

        void LoadFromJson(const nlohmann::json& objJson) override;

    private:
        Mesh* m_mesh = nullptr;
        Material* m_material = nullptr;
        EventHandler m_onTransformDirtyHandler = 0;
        bool m_transformDirty = true;
        BatchMatrix::Elem m_submitBuffer;

        void OnTransformDirty();
        void UpdatePerObjectBuffer();
    };
}
