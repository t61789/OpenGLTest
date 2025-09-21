#include "batch_render_comp.h"

#include <tracy/Tracy.hpp>

#include "game_resource.h"
#include "material.h"
#include "mesh.h"
#include "object.h"
#include "transform_comp.h"
#include "culling_system.h"
#include "render/batch_render_unit.h"

namespace op
{
    void BatchRenderComp::OnEnable()
    {
        m_onTransformDirtyHandler = GetOwner()->transform->dirtyEvent.Add(this, &BatchRenderComp::OnTransformDirty);
        GetGR()->GetBatchRenderUnit()->BindComp(this);

        m_cullingBufferAccessor = GetGR()->GetCullingSystem()->GetCullingBuffer()->Alloc();
    }

    void BatchRenderComp::OnDisable()
    {
        GetOwner()->transform->dirtyEvent.Remove(m_onTransformDirtyHandler);
        GetGR()->GetBatchRenderUnit()->UnBindComp(this);
        
        GetGR()->GetCullingSystem()->GetCullingBuffer()->Release(m_cullingBufferAccessor);
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

        m_worldBounds = m_mesh->GetBounds().ToWorld(GetOwner()->transform->GetLocalToWorld());
        UpdatePerObjectBuffer();
    }

    bool BatchRenderComp::HasONS()
    {
        return GetOwner()->transform->HasOddNegativeScale();
    }

    Bounds BatchRenderComp::GetWorldBounds()
    {
        UpdateTransform();

        return m_worldBounds;
    }

    void BatchRenderComp::UpdatePerObjectBuffer()
    {
        m_submitBuffer.localToWorld = GetOwner()->transform->GetLocalToWorld();
        m_submitBuffer.worldToLocal = GetOwner()->transform->GetWorldToLocal();

        GetGR()->GetBatchRenderUnit()->UpdateMatrix(this, m_submitBuffer);
    }

    void BatchRenderComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("mesh"))
        {
            ZoneScopedN("Load Mesh");
            
            auto meshPath = objJson["mesh"].get<std::string>();
            m_mesh = Mesh::LoadFromFile(meshPath);
        }

        if(objJson.contains("material"))
        {
            ZoneScopedN("Load Material");
            
            auto matPath = objJson["material"].get<std::string>();
            m_material = Material::LoadFromFile(matPath);
        }
    }
}
