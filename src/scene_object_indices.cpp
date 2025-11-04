#include "scene_object_indices.h"

#include "material.h"
#include "object.h"
#include "objects/batch_render_comp.h"
#include "objects/render_comp.h"
#include "objects/transform_comp.h"
#include "render/gl/gl_state.h"

namespace op
{
    umap<std::type_index, CompStorage::CompsAccessor> CompStorage::m_compAccessors = {};
    
    SceneObjectIndices::SceneObjectIndices(crsp<Scene> scene)
    {
        m_scene = scene;
    }

    void SceneObjectIndices::AddObject(crsp<Object> obj)
    {
        assert(!ObjectExists(obj));
        assert(obj->m_scene.lock() != m_scene.lock());
        
        m_objects.push_back(obj);
        obj->m_scene = m_scene;
        for (const auto& comp : obj->GetComps())
        {
            AddComp(comp);
        }

        for (const auto& child : obj->GetChildren())
        {
            AddObject(child);
        }
    }

    void SceneObjectIndices::RemoveObject(crsp<Object> obj)
    {
        assert(ObjectExists(obj));
        assert(obj->m_scene.lock() == m_scene.lock());

        m_objects.erase(
            std::remove_if(
                m_objects.begin(), m_objects.end(),
                [&obj](crwp<Object> x){ return x.lock() == obj; }),
            m_objects.end());
        
        obj->m_scene.reset();
        for (const auto& comp : obj->GetComps())
        {
            RemoveComp(comp);
        }

        for (const auto& child : obj->GetChildren())
        {
            RemoveObject(child);
        }
    }

    void SceneObjectIndices::AddComp(crsp<Comp> comp)
    {
        m_compStorage.AddComp(comp);

        if (auto renderComp = std::dynamic_pointer_cast<RenderComp>(comp))
        {
            RegisterRenderComp(renderComp);
        }
    }

    void SceneObjectIndices::RemoveComp(crsp<Comp> comp)
    {
        m_compStorage.RemoveComp(comp);
        
        if (auto renderComp = std::dynamic_pointer_cast<RenderComp>(comp))
        {
            UnRegisterRenderComp(renderComp);
        }
    }

    sp<Job> SceneObjectIndices::CreateTransparentSortJob()
    {
        assert(!m_transparentSortJob || m_transparentSortJob->IsComplete());
        
        m_transparentSortJob = Job::CreateCommon([this]
        {
            ZoneScopedN("Transparent Sort");
            
            insert_sort(m_transparentComps, [](crwp<RenderComp> x, crwp<RenderComp> y)
            {
                assert(!x.expired() && !y.expired());
                auto cameraPos = GetRC()->camera->GetOwner()->transform->GetWorldPosition();
                auto xPos = x.lock()->GetOwner()->transform->GetWorldPosition();
                auto yPos = y.lock()->GetOwner()->transform->GetWorldPosition();
                return (cameraPos - xPos).Magnitude() >= (cameraPos - yPos).Magnitude();
            });
        });
        return m_transparentSortJob;
    }

    bool SceneObjectIndices::ObjectExists(crsp<Object> obj)
    {
        return exists_if(m_objects, [&obj](crwp<Object> x){ return x.lock() == obj;});
    }

    void SceneObjectIndices::RegisterRenderComp(crsp<RenderComp> comp)
    {
        assert(comp != nullptr);
        auto material = comp->GetMaterial();
        assert(material != nullptr);

        auto& comps = GetRenderComps(material->blendMode);
        assert(!std::any_of(comps.begin(), comps.end(), [&comp](crwp<RenderComp> x){ return x.lock() == comp; }));

        comps.push_back(comp);
    }

    void SceneObjectIndices::UnRegisterRenderComp(crsp<RenderComp> comp)
    {
        assert(comp != nullptr);
        auto material = comp->GetMaterial();
        assert(material != nullptr);

        auto& comps = GetRenderComps(material->blendMode);
        remove_if(comps, [&comp](crwp<RenderComp> x){ return x.lock() == comp; });
    }

    vecwp<RenderComp>& SceneObjectIndices::GetRenderComps(const BlendMode blendMode)
    {
        switch (blendMode)
        {
            case BlendMode::NONE:
            case BlendMode::UNSET:
                return m_opaqueComps;
            case BlendMode::BLEND:
            case BlendMode::ADD:
                return m_transparentComps;
            default:
                throw std::runtime_error("Unsupported blend mode");
        }
    }
}
 