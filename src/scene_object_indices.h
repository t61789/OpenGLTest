#pragma once
#include <typeindex>
#include <unordered_map>

#include "utils.h"
#include "objects/comp.h"

namespace op
{
    class Comp;
    class Object;
    class Scene;

    class CompStorage
    {
        struct CompsAccessor
        {
            std::function<std::any()> createStorageFunc;
            std::function<bool(const std::weak_ptr<Comp>&, std::any&)> addFunc;
            std::function<bool(const std::weak_ptr<Comp>&, std::any&)> removeFunc;
        };

        vecwp<Comp> m_allComps;
        std::unordered_map<std::type_index, std::any> m_comps;
        static std::unordered_map<std::type_index, CompsAccessor> m_compAccessors;
        
    public:
        void AddComp(const std::shared_ptr<Comp>& comp);
        void RemoveComp(const std::shared_ptr<Comp>& comp);
        template <typename T>
        const std::vector<std::weak_ptr<T>>& GetComps();
        crvecwp<Comp> GetAllComps() const { return m_allComps;}
        template <typename T>
        static void RegisterComp();
    };

    class SceneObjectIndices
    {
        friend class Object;
        
    public:
        explicit SceneObjectIndices(crsp<Scene> scene);
        void AddObject(crsp<Object> obj);
        void RemoveObject(crsp<Object> obj);
        void AddComp(crsp<Comp> comp);
        void RemoveComp(crsp<Comp> comp);

        crvecwp<Object> GetAllObjects() const { return m_objects;}
        crvecwp<Comp> GetAllComps() const { return m_compStorage.GetAllComps();}
        
        template <typename T>
        crvecwp<T> GetComps();
        
    private:
        struct CompsAccessor
        {
            std::function<void(crwp<Comp>, std::any&)> addFunc;
            std::function<void(crwp<Comp>, std::any&)> removeFunc;
        };
        
        wp<Scene> m_scene;
        vecwp<Object> m_objects;
        CompStorage m_compStorage;
        
        bool ObjectExists(crsp<Object> obj);
    };

    inline void CompStorage::AddComp(const std::shared_ptr<Comp>& comp)
    {
        const auto& compTypeIndex = comp->GetType();
        auto& compAccessor = m_compAccessors.at(compTypeIndex);

        auto it = m_comps.find(compTypeIndex);
        if (it == m_comps.end())
        {
            m_comps[compTypeIndex] = compAccessor.createStorageFunc();
            it = m_comps.find(compTypeIndex);
        }
            
        if (compAccessor.addFunc(comp, it->second))
        {
            m_allComps.push_back(comp);
        }
    }

    inline void CompStorage::RemoveComp(const std::shared_ptr<Comp>& comp)
    {
        const auto& compTypeIndex = comp->GetType();
        auto& compAccessor = m_compAccessors.at(compTypeIndex);

        auto it = m_comps.find(compTypeIndex);
        if (it == m_comps.end())
        {
            return;
        }
            
        if (compAccessor.removeFunc(comp, it->second))
        {
            op::remove_if(m_allComps, [&comp](crwp<Comp> x)
            {
                return x.lock() == comp;
            });
        }
    }

    template <typename T>
    const std::vector<std::weak_ptr<T>>& CompStorage::GetComps()
    {
        static_assert(std::is_base_of_v<Comp, T>);
            
        std::type_index compTypeIndex = typeid(T);

        auto it = m_comps.find(compTypeIndex);
        if (it == m_comps.end())
        {
            static std::vector<std::weak_ptr<T>> empty;
            return empty;
        }

        return std::any_cast<std::vector<std::weak_ptr<T>>&>(it->second);
    }

    template <typename T>
    void CompStorage::RegisterComp()
    {
        static_assert(std::is_base_of_v<Comp, T>);
            
        std::type_index compTypeIndex = typeid(T);
            
        CompsAccessor compsAccessor;

        compsAccessor.createStorageFunc = []() -> std::any
        {
            return std::vector<std::weak_ptr<T>>();
        };
        
        compsAccessor.addFunc = [](const std::weak_ptr<Comp>& comp, std::any& any) -> bool
        {
            std::weak_ptr<T> p = std::static_pointer_cast<T>(comp.lock());
            auto& v = std::any_cast<std::vector<std::weak_ptr<T>>&>(any);
            auto exists = false;
            for (auto& w : v)
            {
                if (w.lock() == comp.lock())
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
            {
                v.push_back(p);
                return true;
            }

            return false;
        };
        
        compsAccessor.removeFunc = [](const std::weak_ptr<Comp>& comp, std::any& any) -> bool
        {
            std::weak_ptr<T> p = std::static_pointer_cast<T>(comp.lock());
            auto& v = std::any_cast<std::vector<std::weak_ptr<T>>&>(any);

            for (auto i = v.begin(); i != v.end(); ++i)
            {
                if (i->lock() == comp.lock())
                {
                    v.erase(i);
                    return true;
                }
            }

            return false;
        };

        m_compAccessors[compTypeIndex] = std::move(compsAccessor);
    }

    template <typename T>
    crvecwp<T> SceneObjectIndices::GetComps()
    {
        return m_compStorage.GetComps<T>();
    }
}
