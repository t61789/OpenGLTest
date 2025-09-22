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
        m_preHasONS = HasONS();
        
        m_cullingBufferAccessor = GetGR()->GetCullingSystem()->GetCullingBuffer()->Alloc();
        
        GetGR()->GetBatchRenderUnit()->BindComp(this);
        
        m_onTransformDirtyHandler = GetOwner()->transform->dirtyEvent.Add(this, &BatchRenderComp::OnTransformDirty);
        OnTransformDirty();
    }

    void BatchRenderComp::OnDisable()
    {
        GetOwner()->transform->dirtyEvent.Remove(m_onTransformDirtyHandler);
        
        GetGR()->GetBatchRenderUnit()->UnBindComp(this);
        
        GetGR()->GetCullingSystem()->GetCullingBuffer()->Release(m_cullingBufferAccessor);
    }

    void BatchRenderComp::OnTransformDirty()
    {
        if (!IsEnable())
        {
            return;
        }
        
        UpdateTransform();
    }
    
    void BatchRenderComp::UpdateTransform()
    {
        auto hasONS = HasONS();
        if (hasONS != m_preHasONS)
        {
            m_preHasONS = hasONS;

            // When the key changes, rebinding is required for sorting
            // Mesh and Material are included in the key but not yet implemented
            GetGR()->GetBatchRenderUnit()->UnBindComp(this);
            GetGR()->GetBatchRenderUnit()->BindComp(this);
        }
        
        auto worldBounds = m_mesh->GetBounds().ToWorld(GetOwner()->transform->GetLocalToWorld());
        m_cullingBufferAccessor.Submit(worldBounds);
        
        UpdatePerObjectBuffer();
    }

    bool BatchRenderComp::HasONS()
    {
        return GetOwner()->transform->HasOddNegativeScale();
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
