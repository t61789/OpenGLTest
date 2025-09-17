#include "scene_object_indices.h"

#include "object.h"
#include "objects/batch_render_comp.h"

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
    }

    void SceneObjectIndices::RemoveComp(crsp<Comp> comp)
    {
        m_compStorage.RemoveComp(comp);
    }

    bool SceneObjectIndices::ObjectExists(crsp<Object> obj)
    {
        return exists_if(m_objects, [&obj](crwp<Object> x){ return x.lock() == obj;});
    }
}
 