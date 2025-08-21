#include "render_comp.h"

#include "mesh.h"

#include "object.h"
#include "transform_comp.h"
#include "render/per_object_buffer.h"

namespace op
{
    void RenderComp::Awake()
    {
        m_onTransformDirtyHandler = owner->transform->dirtyEvent.Add(this, &RenderComp::OnTransformDirty);
    }

    void RenderComp::Start()
    {
        m_perObjectBufferIndex = GetGR()->perObjectBuffer->BindObject();
    }

    void RenderComp::OnDestroy()
    {
        GetGR()->perObjectBuffer->UnbindObject(m_perObjectBufferIndex);
        
        owner->transform->dirtyEvent.Remove(m_onTransformDirtyHandler);
        
        if (mesh)
        {
            DECREF(mesh);
        }

        if (material)
        {
            DECREF(material);
        }

        if (material)
        {
            DECREF(material);
        }
    }

    void RenderComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("mesh"))
        {
            auto meshPath = objJson["mesh"].get<std::string>();
            mesh = Mesh::LoadFromFile(meshPath);
            if (mesh)
            {
                INCREF(mesh);
            }
        }

        if(objJson.contains("material"))
        {
            auto matPath = objJson["material"].get<std::string>();
            material = Material::LoadFromFile(matPath);
            if (material)
            {
                INCREF(material);
            }
        }
    }

    const Bounds& RenderComp::GetWorldBounds()
    {
        UpdateTransform();

        return m_worldBounds;
    }

    void RenderComp::OnTransformDirty()
    {
        m_transformDirty = true;
    }

    void RenderComp::UpdateTransform()
    {
        if (!m_transformDirty)
        {
            return;
        }
        m_transformDirty = false;

        UpdateWorldBounds();
    }

    void RenderComp::UpdateWorldBounds()
    {
        auto m = owner->transform->GetLocalToWorld();
        const auto& boundsOS = mesh->bounds;
        auto centerWS = Vec3(m * Vec4(boundsOS.center, 1));
        Vec3 extentsWS = {
            dot(abs(Vec3(m[0])), boundsOS.extents),
            dot(abs(Vec3(m[1])), boundsOS.extents),
            dot(abs(Vec3(m[2])), boundsOS.extents)
        };

        m_worldBounds = {centerWS, extentsWS};
    }

    void RenderComp::UpdatePerObjectBuffer()
    {
        m_submitBuffer.localToWorld = owner->transform->GetLocalToWorld();
        m_submitBuffer.worldToLocal = owner->transform->GetWorldToLocal();

        GetGR()->perObjectBuffer->SubmitData(m_perObjectBufferIndex, &m_submitBuffer);
    }
}
