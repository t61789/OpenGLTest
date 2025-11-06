#include "render_comp.h"

#include "game_resource.h"
#include "material.h"
#include "mesh.h"

#include "object.h"
#include "transform_comp.h"
#include "render/per_object_buffer.h"

namespace op
{
    void RenderComp::Awake()
    {
        m_perObjectBufferIndex = GetGR()->GetPerObjectBuffer()->Register();
    }

    void RenderComp::Start()
    {
        m_onTransformDirtyHandler = GetOwner()->transform->dirtyEvent.Add(this, &RenderComp::OnTransformDirty);
        OnTransformDirty();
        UpdateTransform();
    }

    void RenderComp::OnEnable()
    {
        m_transparentCullingBufferAccessor = GetGR()->GetCullingBuffer(CullingGroup::TRANSPARENT)->Alloc();
        OnTransformDirty();
        UpdateTransform();
    }

    void RenderComp::OnDisable()
    {
        if (m_transparentCullingBufferAccessor)
        {
            m_transparentCullingBufferAccessor->Release();
            m_transparentCullingBufferAccessor = nullptr;
        }
    }

    void RenderComp::OnDestroy()
    {
        GetGR()->GetPerObjectBuffer()->UnRegister(m_perObjectBufferIndex);

        if (m_onTransformDirtyHandler)
        {
            GetOwner()->transform->dirtyEvent.Remove(m_onTransformDirtyHandler);
        }
    }

    void RenderComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("mesh"))
        {
            auto meshPath = objJson["mesh"].get<std::string>();
            m_mesh = Mesh::LoadFromFile(meshPath);
        }

        if(objJson.contains("material"))
        {
            auto matPath = objJson["material"].get<std::string>();
            m_material = Material::LoadFromFile(matPath);
        }
    }

    const Bounds& RenderComp::GetWorldBounds()
    {
        UpdateTransform();

        return m_worldBounds;
    }

    bool RenderComp::HasOddNegativeScale() const
    {
        return GetOwner()->transform->HasOddNegativeScale();
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
        UpdatePerObjectBuffer();
    }

    void RenderComp::UpdateWorldBounds()
    {
        m_worldBounds = m_mesh->GetBounds().ToWorld(GetOwner()->transform->GetLocalToWorld());
        if (m_transparentCullingBufferAccessor)
        {
            m_transparentCullingBufferAccessor->Submit(m_worldBounds);
        }
    }

    void RenderComp::UpdatePerObjectBuffer()
    {
        PerObjectBuffer::Elem submitBuffer;
        submitBuffer.localToWorld = GetOwner()->transform->GetLocalToWorld();
        submitBuffer.worldToLocal = GetOwner()->transform->GetWorldToLocal();
        
        GetGR()->GetPerObjectBuffer()->SubmitData(m_perObjectBufferIndex, submitBuffer);
    }
}
