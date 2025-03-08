#include "Object.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include "Utils.h"
#include "glm/gtx/euler_angles.hpp"

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

void Object::Update()
{
    // pass
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
    
