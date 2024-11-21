#pragma once
#include <string>
#include <vec3.hpp>
#include <vector>

#include "glm.hpp"
#include "ResourceMgr.h"

#include "../lib/json.hpp"

typedef RESOURCE_ID OBJECT_ID;

class Object : public ResourceBase
{
public:
    bool enabled = true;
    
    std::string name = "Unnamed object";
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    glm::vec3 rotation = glm::vec3(0, 0, 0);

    std::vector<OBJECT_ID> children;

    glm::mat4 getLocalToWorld() const;
    virtual void update();
    virtual void loadFromJson(const nlohmann::json& objJson);

    void addChild(OBJECT_ID child);
};
