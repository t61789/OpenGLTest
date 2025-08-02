#pragma once
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "event.h"
#include "shared_object.h"
#include "objects/comp.h"

namespace op
{
    class TransformComp;

    class Object : public SharedObject
    {
    public:
        static Object* Create();
        static Object* Create(const std::string& name);
        static Object* CreateFromJson(const nlohmann::json& objJson);
        static Object* Create(const std::string& name, const nlohmann::json& objJson);
        
        ~Object() override;
        
        bool enabled = true;
        
        std::string name = "Unnamed object";

        TransformComp* transform = nullptr;
        Object* parent = nullptr;
        std::vector<Object*> children;
        Event<Object*, Object*> childAddedEvent; // parent, child

        virtual void LoadFromJson(const nlohmann::json& objJson);

        void AddChild(Object* child);
        void RemoveChild(Object* child);
        std::string GetPathInScene() const;

        bool HasComp(const std::string& compName);
        Comp* GetComp(const std::string& compName);
        template <typename T>
        T* GetComp(const std::string& compName)
        {
            return dynamic_cast<T*>(GetComp(compName));
        }
        std::vector<Comp*> GetComps();
        Comp* AddOrCreateComp(const std::string& compName, const nlohmann::json& compJson = nlohmann::json::object())
        {
            auto comp = GetComp<Comp>(compName);
            if (comp)
            {
                return comp;
            }

            auto constructor = GetConstructor(compName);
            assert(constructor);
            if (!constructor)
            {
                return nullptr;
            }

            comp = constructor();
            assert(comp);
            if (!comp)
            {
                return nullptr;
            }

            comp->owner = this;
            m_comps[compName] = comp;

            comp->LoadFromJson(compJson);
            
            comp->Awake();

            return comp;
        }

        template <typename T>
        T* AddOrCreateComp(const std::string& compName, const nlohmann::json& compJson = nlohmann::json::object())
        {
            auto comp = AddOrCreateComp(compName, compJson);
            return dynamic_cast<T*>(comp);
        }

    private:
        Object() = default;
        
        std::unordered_map<std::string, Comp*> m_comps;

        static std::function<Comp*()> GetConstructor(const std::string& name);
        static std::vector<nlohmann::json> GetPresetCompJsons();
        static void LoadCompJsons(std::vector<nlohmann::json>& target, const nlohmann::json& objJson);

        void AddCompsFromJsons(const std::vector<nlohmann::json>& compJsons);
    };
}
