#include "Object.h"

#include "gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include "Utils.h"
#include "gtx/euler_angles.hpp"

glm::vec3 ToVec3(nlohmann::json arr)
{
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
}

glm::mat4 Object::getLocalToWorld() const
{
    auto objectMatrix = glm::mat4(1);
    objectMatrix = translate(objectMatrix, position);
    objectMatrix = objectMatrix * glm::eulerAngleYXZ(
        glm::radians(rotation.y),
        glm::radians(rotation.x),
        glm::radians(rotation.z));
    objectMatrix = glm::scale(objectMatrix, scale);
    return objectMatrix;
}

void Object::update()
{
    // pass
}

void Object::loadFromJson(const nlohmann::json& objJson)
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

void Object::addChild(const OBJECT_ID child)
{
    if(std::find(children.begin(), children.end(), child) != children.end())
    {
        return;
    }

    children.push_back(child);
}
