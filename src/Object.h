#pragma once
#include <string>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "Event.h"
#include "SharedObject.h"


class Object : public SharedObject
{
public:
    ~Object() override;
    
    bool enabled = true;
    
    std::string name = "Unnamed object";
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    glm::vec3 rotation = glm::vec3(0, 0, 0);

    Object* parent = nullptr;
    std::vector<Object*> children;
    typedef std::function<void(Object* parent, Object* child)> ChildAddedCallback;
    Event<void(Object* parent, Object* child)> childAddedEvent;

    glm::mat4 GetLocalToWorld() const;
    virtual void Update();
    virtual void LoadFromJson(const nlohmann::json& objJson);

    void AddChild(Object* child);
    void RemoveChild(Object* child);
    std::string GetPathInScene() const;
};
