#include "scene_object_indices.h"

namespace op
{
    void SceneObjectIndices::AddObject(Object* obj)
    {
        m_objects.push_back(obj);
        for (auto comp : obj->GetComps())
        {
            AddComp(comp->GetName(), comp);
        }
    }

    void SceneObjectIndices::RemoveObject(Object* obj)
    {
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

    void SceneObjectIndices::AddComp(const std::string& compName, Comp* comp)
    {
        if (m_comps.find(compName) == m_comps.end())
        {
            m_comps[compName] = std::vector<Comp*>();
        }

        m_comps[compName].push_back(comp);
    }

    void SceneObjectIndices::RemoveComp(const std::string& compName, Comp* comp)
    {
        if (m_comps.find(compName) == m_comps.end())
        {
            return;
        }

        m_comps[compName].erase(std::remove(m_comps[compName].begin(), m_comps[compName].end(), comp), m_comps[compName].end());
    }
}
