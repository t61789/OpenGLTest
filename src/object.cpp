#include "object.h"

#include "scene.h"
#include "utils.h"
#include "objects/camera_comp.h"
#include "objects/light_comp.h"
#include "objects/render_comp.h"
#include "objects/runtime_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    Object* Object::Create()
    {
        return Create("Unnamed object", nlohmann::json::object());
    }

    Object* Object::Create(const std::string& name)
    {
        return Create(name, nlohmann::json::object());
    }
    
    Object* Object::CreateFromJson(const nlohmann::json& objJson)
    {
        return Create("Unnamed object", objJson);
    }
    
    Object* Object::Create(const std::string& name, const nlohmann::json& objJson)
    {
        auto result = new Object();
        result->name = name;
        result->LoadFromJson(objJson);
        return result;
    }

    Object::~Object()
    {
        for (auto child : children)
        {
            DECREF(child);
        }
        
        for (const auto& comp : m_comps)
        {
            comp.second->OnDestroy();
        }

        for (const auto& comp : m_comps)
        {
            delete comp.second;
        }
    }

    void Object::LoadFromJson(const nlohmann::json& objJson)
    {
        std::vector<nlohmann::json> comps = GetPresetCompJsons();
        LoadCompJsons(comps, objJson);
        AddCompsFromJsons(comps);
        
        if(objJson.contains("name"))
        {
            name = objJson["name"].get<std::string>();
        }
    }

    void Object::AddChild(Object* child)
    {
        if(std::find(children.begin(), children.end(), child) != children.end())
        {
            return;
        }

        child->parent = this;
        child->scene = scene;
        scene->objectIndices->AddObject(child);
        children.push_back(child);
        INCREF(child);
    }

    void Object::RemoveChild(Object* child)
    {
        auto it = std::find(children.begin(), children.end(), child);
        if(it == children.end())
        {
            return;
        }

        child->parent = nullptr;
        child->scene = nullptr;
        scene->objectIndices->RemoveObject(child);
        children.erase(it);
        DECREF(child);
    }

    std::string Object::GetPathInScene() const
    {
        auto path = std::vector<std::string>();
        auto curObj = this;

        while (curObj)
        {
            path.push_back(curObj->name);
            curObj = curObj->parent;
        }

        return join(path, "/");
    }

    bool Object::HasComp(const std::string& compName)
    {
        return m_comps.find(compName) != m_comps.end();
    }

    Comp* Object::GetComp(const std::string& compName)
    {
        Comp* result = nullptr;
        auto it = m_comps.find(compName);
        if (it != m_comps.end())
        {
            result = it->second;
        }

        return result;
    }

    std::vector<Comp*> Object::GetComps()
    {
        auto result = std::vector<Comp*>();
        result.reserve(m_comps.size());
        for (auto& pair : m_comps)
        {
            result.push_back(pair.second);
        }
        return result;
    }

    std::function<Comp*()> Object::GetConstructor(const std::string& name)
    {
        static std::unordered_map<std::string, std::function<Comp*()>> constructors;
        
        if (constructors.empty())
        {
    #define REGISTER_OBJECT(t) constructors[#t] = []() -> Comp* { auto result = new t(); result->SetName(#t); return result; }

            REGISTER_OBJECT(RenderComp);
            REGISTER_OBJECT(LightComp);
            REGISTER_OBJECT(CameraComp);
            REGISTER_OBJECT(TransformComp);
            REGISTER_OBJECT(RuntimeComp);
            
    #undef REGISTER_OBJECT
        }

        auto it = constructors.find(name);
        if (it != constructors.end())
        {
            return it->second;
        }

        return []() -> Comp* { return nullptr; };
    }

    std::vector<nlohmann::json> Object::GetPresetCompJsons()
    {
        std::vector<nlohmann::json> result;
        result.push_back({
            {"name", "TransformComp"}
        });

        return result;
    }

    void Object::LoadCompJsons(std::vector<nlohmann::json>& target, const nlohmann::json& objJson)
    {
        if (!objJson.contains("comps"))
        {
            return;
        }
        
        for (const auto& compFromFile : objJson["comps"])
        {
            auto compName = compFromFile["name"].get<std::string>();

            // 如果文件里的这个组件在结果list中已经存在了，就合并到结果中的对应组件，否则就添加它
            auto it = std::find_if(target.begin(), target.end(), [&compName](const nlohmann::json& compJson) {
                return compJson["name"].get<std::string>() == compName;
            });

            if (it == target.end())
            {
                target.push_back(compFromFile);
                continue;
            }

            Utils::MergeJson(*it, compFromFile);
        }
    }
    
    void Object::AddCompsFromJsons(const std::vector<nlohmann::json>& compJsons)
    {
        for (auto& compJson : compJsons)
        {
            auto compName = compJson["name"].get<std::string>();
            auto comp = GetConstructor(compName);
            assert(comp);
            if (!comp)
            {
                continue;
            }

            AddOrCreateComp(compName, compJson);
        }
    }
}
