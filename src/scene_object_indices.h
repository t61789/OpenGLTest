#pragma once
#include <string>
#include <unordered_map>

#include "object.h"
#include "objects/comp.h"

namespace op
{
    class SceneObjectIndices
    {
    public:
        void AddObject(Object* obj);
        void RemoveObject(Object* obj);
        std::vector<Object*>* GetAllObjects();
        void AddComp(const std::string& compName, Comp* comp);
        void RemoveComp(const std::string& compName, Comp* comp);
        template <typename T>
        std::vector<T*>* GetAllComps(const std::string& compName)
        {
            if (m_comps.find(compName) == m_comps.end())
            {
                throw std::runtime_error("Comp not found");
            }

            return reinterpret_cast<std::vector<T*>*>(&m_comps[compName]);
        }
        
    private:
        std::vector<Object*> m_objects;
        std::unordered_map<std::string, std::vector<Comp*>> m_comps;
    };
}
