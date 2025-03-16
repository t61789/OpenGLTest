#include "Object.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/euler_angles.hpp"

#include "Utils.h"
#include "Objects/CameraComp.h"
#include "Objects/LightComp.h"
#include "Objects/RenderComp.h"
#include "Objects/RuntimeComp.h"
#include "Objects/TransformComp.h"

glm::vec3 ToVec3(nlohmann::json arr)
{
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
}

Object::Object()
{
    transform = AddComp<TransformComp>("TransformComp");
}

Object::Object(const std::string& name)
{
    this->name = name;
    transform = AddComp<TransformComp>("TransformComp");
}

Object::~Object()
{
    for (auto child : children)
    {
        DECREF(child);
    }

    for (const auto& comp : m_comps)
    {
        delete comp.second;
    }
}

void Object::LoadFromJson(const nlohmann::json& objJson)
{
    if(objJson.contains("name"))
    {
        name = objJson["name"].get<std::string>();
    }

    if (objJson.contains("comps"))
    {
        auto arr = objJson["comps"];
        for (auto& compJson : arr)
        {
            auto compName = compJson["name"].get<std::string>();

            // 如果这个组件已经加载过，就读一遍参数
            // 没加载过就加载，找不到这个组件的构造函数就不管了

            auto comp = GetComp(compName);
            if (!comp)
            {
                comp = GetConstructor(compName)();
                if (!comp)
                {
                    continue;
                }
                
                comp->owner = this;
                m_comps[compName] = comp;
            }

            comp->LoadFromJson(compJson);
        }
    }
}

void Object::AddChild(Object* child)
{
    if(std::find(children.begin(), children.end(), child) != children.end())
    {
        return;
    }

    child->parent = this;
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

    return Utils::JoinStrings(std::vector<std::string>(path.rbegin(), path.rend()), "/");
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
#define REGISTER_OBJECT(t) constructors[#t] = []() -> Comp* { return new t(); }

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
