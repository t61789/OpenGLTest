#include "batch_render_comp.h"

#include "material.h"
#include "mesh.h"
#include "object.h"
#include "transform_comp.h"
#include "render/batch_render_unit.h"

namespace op
{
    void BatchRenderComp::OnEnable()
    {
        m_onTransformDirtyHandler = GetOwner()->transform->dirtyEvent.Add(this, &BatchRenderComp::OnTransformDirty);
        BatchRenderUnit::Ins()->BindComp(this);
    }

    void BatchRenderComp::OnDisable()
    {
        BatchRenderUnit::Ins()->UnBindComp(this);
    }

    void BatchRenderComp::OnTransformDirty()
    {
        m_transformDirty = true;
    }
    
    void BatchRenderComp::UpdateTransform()
    {
        if (!m_transformDirty)
        {
            return;
        }
        m_transformDirty = false;

        UpdatePerObjectBuffer();
    }
    
    void BatchRenderComp::UpdatePerObjectBuffer()
    {
        m_submitBuffer.localToWorld = GetOwner()->transform->GetLocalToWorld();
        m_submitBuffer.worldToLocal = GetOwner()->transform->GetWorldToLocal();

        BatchRenderUnit::Ins()->UpdateMatrix(this, &m_submitBuffer);
    }

    void BatchRenderComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("mesh"))
        {
            auto meshPath = objJson["mesh"].get<std::string>();
            m_mesh = Mesh::LoadFromFile(meshPath);
            INCREF(m_mesh)
        }

        if(objJson.contains("material"))
        {
            auto matPath = objJson["material"].get<std::string>();
            m_material = Material::LoadFromFile(matPath);
            INCREF(m_material)
        }
    }
}
