#pragma once
#include <string>
#include <vec3.hpp>
#include <vector>

#include "glm.hpp"
#include "ResourceMgr.h"

#include "../lib/json.hpp"

typedef RESOURCE_ID OBJECT_ID;

class Object
{
public:
    OBJECT_ID m_id;

    bool m_enabled = true;
    
    std::string m_name = "Unnamed object";
    glm::vec3 m_position = glm::vec3(0, 0, 0);
    glm::vec3 m_scale = glm::vec3(1, 1, 1);
    glm::vec3 m_rotation = glm::vec3(0, 0, 0);

    std::vector<OBJECT_ID> m_children;

    Object();
    Object(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);
    virtual ~Object();

    glm::mat4 GetLocalToWorld() const;
    virtual void Update();
    virtual void LoadFromJson(const nlohmann::json& objJson);

    void AddChild(OBJECT_ID child);
};
