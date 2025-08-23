#include "scene_object_indices.h"

namespace op
{
    SceneObjectIndices::SceneObjectIndices(Scene* scene)
    {
        this->scene = scene;
    }

    void SceneObjectIndices::AddObject(Object* obj)
    {
        obj->scene = scene;
        m_objects.push_back(obj);
        for (auto comp : obj->GetComps())
        {
            AddComp(comp->GetName(), comp);
        }
    }

    void SceneObjectIndices::RemoveObject(Object* obj)
    {
        obj->scene = nullptr;
        m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), obj), m_objects.end());
        for (auto comp : obj->GetComps())
        {
            RemoveComp(comp->GetName(), comp);
        }
    }

    std::vector<Object*>* SceneObjectIndices::GetAllObjects()
    {
        return &m_objects;
    }

    void SceneObjectIndices::AddComp(const string_hash& compNameId, Comp* comp)
    {
        comp->SetScene(scene);
        
        if (m_comps.find(compNameId) == m_comps.end())
        {
            m_comps[compNameId] = std::vector<Comp*>();
        }

        m_comps[compNameId].push_back(comp);
    }

    void SceneObjectIndices::RemoveComp(const string_hash& compNameId, Comp* comp)
    {
        comp->SetScene(nullptr);
        
        if (m_comps.find(compNameId) == m_comps.end())
        {
            return;
        }

        m_comps[compNameId].erase(std::remove(m_comps[compNameId].begin(), m_comps[compNameId].end(), comp), m_comps[compNameId].end());
    }
}
