#pragma once
#include <string>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "event.h"
#include "shared_object.h"
#include "objects/comp.h"

namespace op
{
    class TransformComp;

    class Object : public SharedObject
    {
    public:
        Object();
        explicit Object(const std::string& name);
        ~Object() override;
        
        bool enabled = true;
        
        std::string name = "Unnamed object";

        TransformComp* transform = nullptr;
        Object* parent = nullptr;
        std::vector<Object*> children;
        typedef std::function<void(Object* parent, Object* child)> ChildAddedCallback;
        Event<void(Object* parent, Object* child)> childAddedEvent;

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
        template <typename T>
        T* AddComp(const std::string& compName)
        {
            auto comp = GetComp<T>(compName);
            if (comp)
            {
                return comp;
            }

            comp = dynamic_cast<T*>(GetConstructor(compName)());
            if (!comp)
            {
                return nullptr;
            }

            comp->owner = this;
            m_comps[compName] = comp;
            return comp;
        }

    private:
        std::unordered_map<std::string, Comp*> m_comps;

        static std::function<Comp*()> GetConstructor(const std::string& name);
    };
}
