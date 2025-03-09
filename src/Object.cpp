#include "Object.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/euler_angles.hpp"

#include "Utils.h"
#include "Objects/CameraComp.h"
#include "Objects/LightComp.h"
#include "Objects/RenderComp.h"

glm::vec3 ToVec3(nlohmann::json arr)
{
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
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

glm::mat4 Object::GetLocalToWorld() const
{
    // TODO 进行缓存
    auto objectMatrix = glm::mat4(1);
    objectMatrix = translate(objectMatrix, position);
    objectMatrix = objectMatrix * glm::eulerAngleYXZ(
        glm::radians(rotation.y),
        glm::radians(rotation.x),
        glm::radians(rotation.z));
    objectMatrix = glm::scale(objectMatrix, scale);
    return objectMatrix;
}

void Object::LoadFromJson(const nlohmann::json& objJson)
{
    if(objJson.contains("name"))
    {
        name = objJson["name"].get<std::string>();
    }
    
    if(objJson.contains("position"))
    {
        position = Utils::ToVec3(objJson["position"]);
    }
    
    if(objJson.contains("rotation"))
    {
        rotation = Utils::ToVec3(objJson["rotation"]);
    }
    
    if(objJson.contains("scale"))
    {
        scale = Utils::ToVec3(objJson["scale"]);
    }

    if (objJson.contains("comps"))
    {
        auto arr = objJson["comps"];
        for (auto& compJson : arr)
        {
            auto compName = compJson["name"].get<std::string>();
            auto comp = GetConstructor(compName)();
            if (!comp)
            {
                continue;
            }

            if (this->name.find("Camera") != std::string::npos)
            {
                Utils::LogInfo("123");
            }
            
            comp->owner = this;
            comp->LoadFromJson(compJson);
            m_comps[compName] = comp;
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
#define REGISTER_OBJECT(n, t) constructors[n] = []() -> Comp* { return new t(); }

        REGISTER_OBJECT("RenderComp", RenderComp);
        REGISTER_OBJECT("LightComp", LightComp);
        REGISTER_OBJECT("CameraComp", CameraComp);
        
#undef REGISTER_OBJECT
    }

    auto it = constructors.find(name);
    if (it != constructors.end())
    {
        return it->second;
    }

    return []() -> Comp* { return nullptr; };
}
