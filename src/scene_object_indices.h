#pragma once
#include <string>
#include <unordered_map>

#include "object.h"
#include "objects/comp.h"
#include "utils.h"

namespace op
{
    class Scene;
    
    class SceneObjectIndices
    {
    public:
        SceneObjectIndices(Scene* scene);
        void AddObject(Object* obj);
        void RemoveObject(Object* obj);
        std::vector<Object*>* GetAllObjects();
        void AddComp(const string_hash& compNameId, Comp* comp);
        void RemoveComp(const string_hash& compNameId, Comp* comp);
        std::unordered_map<string_hash, std::vector<Comp*>>* GetAllComps() { return &m_comps; }
        template <typename T>
        std::vector<T*>* GetAllComps(const string_hash& compNameId)
        {
            if (m_comps.find(compNameId) == m_comps.end())
            {
                THROW_ERROR("Comp not found")
            }

            return reinterpret_cast<std::vector<T*>*>(&m_comps[compNameId]);
        }
        
    private:
        Scene* scene = nullptr;
        std::vector<Object*> m_objects;
        std::unordered_map<string_hash, std::vector<Comp*>> m_comps;
    };
}
