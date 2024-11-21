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

glm::mat4 Object::GetLocalToWorld() const
{
    auto objectMatrix = glm::mat4(1);
    objectMatrix = translate(objectMatrix, m_position);
    objectMatrix = objectMatrix * glm::eulerAngleYXZ(
        glm::radians(m_rotation.y),
        glm::radians(m_rotation.x),
        glm::radians(m_rotation.z));
    objectMatrix = glm::scale(objectMatrix, m_scale);
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
        m_name = objJson["name"].get<std::string>();
    }
    if(objJson.contains("position"))
    {
        m_position = Utils::ToVec3(objJson["position"]);
    }
    if(objJson.contains("rotation"))
    {
        m_rotation = Utils::ToVec3(objJson["rotation"]);
    }
    if(objJson.contains("scale"))
    {
        m_scale = Utils::ToVec3(objJson["scale"]);
    }
}

void Object::AddChild(const OBJECT_ID child)
{
    if(std::find(m_children.begin(), m_children.end(), child) != m_children.end())
    {
        return;
    }

    m_children.push_back(child);
}
