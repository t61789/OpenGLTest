#pragma once
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "event.h"
#include "objects/comp.h"
#include "objects/camera_comp.h"

namespace op
{
    class TransformComp;
    class Scene;

    class Object final : public std::enable_shared_from_this<Object>
    {
        friend class Scene;
        friend class SceneObjectIndices;
        
    public:
        bool enabled = true;
        
        StringHandle name = UNNAMED_OBJECT;

        TransformComp* transform = nullptr;
        wp<Object> parent;

        Object() = default;

        void LoadFromJson(const nlohmann::json& objJson);

        void AddChild(crsp<Object> child);
        void RemoveChild(crsp<Object> child);
        crvecsp<Object> GetChildren() const { return m_children;}
        std::string GetPathInScene() const;

        bool HasComp(cr<StringHandle> compName);
        sp<Comp> GetComp(cr<StringHandle> compName);
        sp<Comp> AddOrCreateComp(cr<StringHandle> compName, const nlohmann::json& compJson = nlohmann::json::object());
        crvecsp<Comp> GetComps();
        
        template <typename T>
        sp<T> GetComp(cr<StringHandle> compName);
        template <typename T>
        sp<T> AddOrCreateComp(cr<StringHandle> compName, const nlohmann::json& compJson = nlohmann::json::object());
        
        static sp<Object> Create(cr<StringHandle> name = UNNAMED_OBJECT);
        static sp<Object> CreateFromJson(const nlohmann::json& objJson);

    private:
        wp<Scene> m_scene;
        vecsp<Comp> m_comps;
        vecsp<Object> m_children;
        
        static std::unordered_map<string_hash, std::function<sp<Comp>()>> m_compConstructors;

        void AddCompsFromJsons(const std::vector<nlohmann::json>& compJsons);

        static void InitComps();
        static const std::function<sp<Comp>()>& GetCompConstructor(cr<StringHandle> compNameId);
        static std::vector<nlohmann::json> GetPresetCompJsons();
        static void LoadCompJsons(std::vector<nlohmann::json>& target, const nlohmann::json& objJson);
    };
        
    template <typename T>
    sp<T> Object::GetComp(cr<StringHandle> compName)
    {
        return std::dynamic_pointer_cast<T>(GetComp(compName));
    }
    
    template <typename T>
    sp<T> Object::AddOrCreateComp(cr<StringHandle> compName, const nlohmann::json& compJson)
    {
        return std::dynamic_pointer_cast<T>(AddOrCreateComp(compName, compJson));
    }
}
