﻿#include "Object.h"

#include "gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "Utils.h"
#include "gtx/euler_angles.hpp"

std::vector<Object*> Object::s_objs;

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
    m_name = "Unnamed object";
    m_id = s_objs.size();
    s_objs.push_back(this);
    m_position = glm::vec3(0, 0, 0);
    m_scale = glm::vec3(1, 1, 1);
    m_rotation = glm::vec3(0, 0, 0);
}

Object::Object(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation):
m_position(position),
m_scale(scale),
m_rotation(rotation)
{
    m_name = "Unnamed object";
    m_id = s_objs.size();
    s_objs.push_back(this);
}

Object::~Object()
{
    s_objs[m_id] = nullptr;
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
        m_position = Utils::ToVec3(objJson["rotation"]);
    }
    if(objJson.contains("scale"))
    {
        m_position = Utils::ToVec3(objJson["scale"]);
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

Object* Object::GetObjectPtr(const OBJECT_ID id)
{
    if(id < 0 || id >= s_objs.size())
    {
        return nullptr;
    }

    return s_objs[id];
}
