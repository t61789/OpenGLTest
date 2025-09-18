#include "object.h"

#include <memory.h>

#include "const.h"
#include "scene.h"
#include "scene_object_indices.h"
#include "utils.h"
#include "objects/batch_render_comp.h"
#include "objects/camera_comp.h"
#include "objects/light_comp.h"
#include "objects/render_comp.h"
#include "objects/runtime_comp.h"
#include "objects/transform_comp.h"

namespace op
{
    std::unordered_map<string_hash, std::function<sp<Comp>()>> Object::m_compConstructors;
    
    sp<Object> Object::Create(const StringHandle& name)
    {
        auto result = msp<Object>();
        result->name = name;
        result->LoadFromJson(nlohmann::json::object());
        return result;
    }
    
    sp<Object> Object::CreateFromJson(const nlohmann::json& objJson)
    {
        auto result = msp<Object>();
        result->LoadFromJson(objJson);
        return result;
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

    void Object::AddChild(crsp<Object> child)
    {
        if(exists(m_children, child))
        {
            return;
        }

        auto curObj = parent.lock();
        while (curObj)
        {
            if (curObj == child)
            {
                log_warning("Can not add a child that is any of its superior nodes");
                return; // 这个child不能是自己上级的任何一个节点
            }

            curObj = curObj->parent.lock();
        }

        // child有parent的话先解绑
        if (!child->parent.expired())
        {
            child->parent.lock()->RemoveChild(child);
        }
        child->parent = shared_from_this();
        
        if (!m_scene.expired())
        {
            m_scene.lock()->GetIndices()->AddObject(child);
        }
        
        m_children.push_back(child);
    }

    void Object::RemoveChild(crsp<Object> child)
    {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if(it == m_children.end())
        {
            return;
        }

        assert(child->parent.lock() == shared_from_this());

        child->parent.reset();
        
        if (!m_scene.expired())
        {
            m_scene.lock()->GetIndices()->RemoveObject(child);
        }
        m_children.erase(it);
    }

    std::string Object::GetPathInScene() const
    {
        vec<std::string> path;
        
        auto curObj = shared_from_this();
        while (curObj)
        {
            path.push_back(curObj->name);
            curObj = curObj->parent.lock();
        }

        return join(path, "/");
    }

    bool Object::HasComp(cr<StringHandle> compName)
    {
        return GetComp(compName) != nullptr;
    }

    sp<Comp> Object::GetComp(cr<StringHandle> compName)
    {
        auto result = find_if(m_comps, [compName](crsp<Comp> x)
        {
            return x->m_name == compName;
        });
        
        if (!result)
        {
            return nullptr;
        }
        return *result;
    }

    crvecsp<Comp> Object::GetComps()
    {
        return m_comps;
    }

    const std::function<sp<Comp>()>& Object::GetCompConstructor(cr<StringHandle> compNameId)
    {
        InitComps();

        return m_compConstructors.at(compNameId);
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
            AddOrCreateComp(StringHandle(compName), compJson);
        }
    }

    void Object::InitComps()
    {
        if (!m_compConstructors.empty())
        {
            return;
        }

        #define REGISTER_COMP(t) \
            m_compConstructors[StringHandle(#t)] = []() -> sp<Comp> { \
                auto result = std::make_shared<t>(); \
                result->SetName(StringHandle(#t)); \
                result->SetType(std::type_index(typeid(t))); \
                return result; \
            }; \
            CompStorage::RegisterComp<t>();

        REGISTER_COMP(RenderComp)
        REGISTER_COMP(TransformComp)
        REGISTER_COMP(CameraComp)
        REGISTER_COMP(LightComp)
        REGISTER_COMP(RuntimeComp)
        REGISTER_COMP(BatchRenderComp)

        #undef REGISTER_COMP
    }

    // todo 改成泛型
    sp<Comp> Object::AddOrCreateComp(cr<StringHandle> compName, const nlohmann::json& compJson)
    {
        auto comp = GetComp<Comp>(compName);
        if (comp)
        {
            return comp;
        }

        comp = GetCompConstructor(compName)();
        if (!comp)
        {
            return nullptr;
        }

        comp->m_owner = this;
        m_comps.push_back(comp);
        if (!m_scene.expired())
        {
            m_scene.lock()->GetIndices()->AddComp(comp);
        }

        comp->LoadFromJson(compJson);

        comp->Awake();

        return comp;
    }
}
