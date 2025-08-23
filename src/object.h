#pragma once
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "event.h"
#include "shared_object.h"
#include "objects/comp.h"
#include "objects/camera_comp.h"

namespace op
{
    class TransformComp;
    class Scene;

    class Object : public SharedObject
    {
    public:
        static Object* Create(const StringHandle& name = UNNAMED_OBJECT);
        static Object* CreateFromJson(const nlohmann::json& objJson);

        ~Object() override;
        
        bool enabled = true;
        
        StringHandle name = UNNAMED_OBJECT;

        TransformComp* transform = nullptr;
        Object* parent = nullptr;
        Scene* scene = nullptr;
        std::vector<Object*> children;
        Event<Object*, Object*> childAddedEvent; // parent, child

        void LoadFromJson(const nlohmann::json& objJson);

        void AddChild(Object* child);
        void RemoveChild(Object* child);
        std::string GetPathInScene() const;

        bool HasComp(string_hash compNameId);
        Comp* GetComp(string_hash compNameId);
        Comp* AddOrCreateComp(string_hash compNameId, const nlohmann::json& compJson = nlohmann::json::object());
        const std::vector<Comp*>& GetComps();
        
        template <typename T>
        T* GetComp(string_hash compNameId);
        template <typename T>
        T* AddOrCreateComp(string_hash compNameId, const nlohmann::json& compJson = nlohmann::json::object());

    private:
        Object() = default;
        
        std::vector<Comp*> m_comps;

        static const std::function<Comp*()>& GetCompConstructor(const string_hash& compNameId);
        static std::vector<nlohmann::json> GetPresetCompJsons();
        static void LoadCompJsons(std::vector<nlohmann::json>& target, const nlohmann::json& objJson);

        void AddCompsFromJsons(const std::vector<nlohmann::json>& compJsons);
    };
        
    template <typename T>
    T* Object::GetComp(const size_t compNameId)
    {
        return dynamic_cast<T*>(GetComp(compNameId));
    }
    template <typename T>
    T* Object::AddOrCreateComp(const size_t compNameId, const nlohmann::json& compJson)
    {
        return dynamic_cast<T*>(AddOrCreateComp(compNameId, compJson));
    }
}
