#pragma once
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "SharedObject.h"

#include "json.hpp"

class Object : public SharedObject
{
public:
    ~Object() override;
    
    bool enabled = true;
    
    std::string name = "Unnamed object";
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    glm::vec3 rotation = glm::vec3(0, 0, 0);

    std::vector<Object*> children;

    glm::mat4 GetLocalToWorld() const;
    virtual void Update();
    virtual void LoadFromJson(const nlohmann::json& objJson);

    void AddChild(Object* child);
    void RemoveChild(Object* child);
};
