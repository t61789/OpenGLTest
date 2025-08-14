#include "render_comp.h"

#include "mesh.h"
#include "material.h"
#include "object.h"
#include "transform_comp.h"

namespace op
{
    void RenderComp::Awake()
    {
        m_onTransformDirtyHandler = owner->transform->dirtyEvent.Add(this, &RenderComp::OnTransformDirty);
    }

    void RenderComp::Start()
    {
        
    }

    void RenderComp::OnDestroy()
    {
        owner->transform->dirtyEvent.Remove(m_onTransformDirtyHandler);
        
        if (mesh)
        {
            DECREF(mesh);
        }

        if (material)
        {
            DECREF(material);
        }

        if (materialNew)
        {
            DECREF(materialNew);
        }
    }

    void RenderComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if(objJson.contains("mesh"))
        {
            auto meshPath = objJson["mesh"].get<std::string>();
            if (meshPath.find("beast") != std::string::npos)
            {
                mesh = Mesh::LoadFromFile0(meshPath);
            }
            else
            {
                mesh = Mesh::LoadFromFile(objJson["mesh"].get<std::string>());
            }
            if (mesh)
            {
                INCREF(mesh);
            }
        }

        if(objJson.contains("material"))
        {
            auto matPath = objJson["material"].get<std::string>();
            if (matPath.find("lit_mat") != std::string::npos)
            {
                materialNew = MaterialNew::LoadFromFile(matPath); // TODO new material
                if (materialNew)
                {
                    INCREF(materialNew);
                }
            }
            else
            {
                material = Material::LoadFromFile(matPath);
                if (material)
                {
                    INCREF(material);
                }
            }
        }
    }

    const Bounds& RenderComp::GetWorldBounds()
    {
        UpdateWorldBounds();

        return m_worldBounds;
    }

    void RenderComp::OnTransformDirty()
    {
        m_transformDirty = true;
    }

    void RenderComp::UpdateWorldBounds()
    {
        if (!m_transformDirty)
        {
            return;
        }
        m_transformDirty = false;
        
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
}
