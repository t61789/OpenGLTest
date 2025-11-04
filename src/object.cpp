#include "object.h"

#include "const.h"
#include "game_framework.h"
#include "game_resource.h"
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
    
    sp<Object> Object::Create(cr<StringHandle> name, crsp<Object> parent)
    {
        auto result = msp<Object>();
        result->name = name;

        if (parent)
        {
            result->SetParent(parent);
        }
        else
        {
            result->SetParent(GetGR()->GetMainScene()->GetRoot());
        }
        
        result->LoadFromJson(nlohmann::json::object());
        
        return result;
    }
    
    sp<Object> Object::CreateFromJson(const nlohmann::json& objJson, crsp<Object> parent)
    {
        auto result = msp<Object>();
        
        if (parent)
        {
            result->SetParent(parent);
        }
        else
        {
            result->SetParent(GetGR()->GetMainScene()->GetRoot());
        }
        
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

    void Object::SetEnable(const bool enable)
    {
        if (enable && !m_enable)
        {
            m_enable = true;

            UpdateRealEnable();
        }
        else if (!enable && m_enable)
        {
            m_enable = false;

            UpdateRealEnable();
        }
    }

    void Object::SetParent(crsp<Object> obj)
    {
        auto newParent = obj ? obj : GetGR()->GetMainScene()->GetRoot();
        
        if (newParent == this->parent.lock())
        {
            return;
        }

        auto curObj = newParent.get();
        while (curObj)
        {
            if (curObj == this)
            {
                log_warning("The current node cannot be any ancestor of the target node");
                return;
            }

            curObj = curObj->parent.lock().get();
        }

        // Remove from the old parent when parent exists
        if (!parent.expired())
        {
            remove(parent.lock()->m_children, shared_from_this());
            parent.reset();
        }
        parent = newParent;

        assert(!exists(obj->m_children, shared_from_this()));

        obj->m_children.push_back(shared_from_this());
        if (obj->m_scene.lock() != m_scene.lock())
        {
            if (!m_scene.expired())
            {
                m_scene.lock()->GetIndices()->RemoveObject(shared_from_this());
            }

            if (!obj->m_scene.expired())
            {
                obj->m_scene.lock()->GetIndices()->AddObject(shared_from_this());
            }

            m_scene = obj->m_scene;
        }
    }

    void Object::Destroy()
    {
        // GameFramework::Ins()->EnqueueDestroyedObject(shared_from_this());

        auto self = shared_from_this();

        auto tempChildren = m_children;
        for (auto& child : tempChildren)
        {
            child->Destroy();
        }
        
        if (!parent.expired())
        {
            remove(parent.lock()->m_children, shared_from_this());
            parent.reset();
        }

        if (!m_scene.expired())
        {
            m_scene.lock()->GetIndices()->RemoveObject(shared_from_this());
        }
        
        for (auto& comp: m_comps)
        {
            comp->Destroy();
        }

        tempChildren.clear();
    }

    std::string Object::GetPathInScene() const
    {
        vec<std::string> path;
        
        auto curObj = this;
        while (curObj)
        {
            path.push_back(curObj->name);
            curObj = curObj->parent.lock().get();
        }

        return join(path, "/");
    }

    void Object::UpdateRealEnable()
    {
        m_realEnable = m_enable;
        if (!parent.expired())
        {
            m_realEnable = m_realEnable && parent.lock()->IsEnable();
        }

        for (auto& comp: m_comps)
        {
            comp->UpdateRealEnable();
        }

        for (auto& child : m_children)
        {
            child->UpdateRealEnable();
        }
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
                result->m_name = StringHandle(#t); \
                result->m_type = std::type_index(typeid(t)); \
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
        comp->LoadFromJson(compJson);

        comp->m_owner = this;
        m_comps.push_back(comp);
        if (!m_scene.expired())
        {
            m_scene.lock()->GetIndices()->AddComp(comp);
        }

        comp->Awake();
        comp->SetEnable(true);

        return comp;
    }
}
