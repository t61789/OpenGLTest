#pragma once
#include <string>
#include <vec3.hpp>
#include <vector>

#include "glm.hpp"

#include "../lib/json.hpp"

typedef unsigned long long OBJECT_ID;

class Object
{
public:
    OBJECT_ID m_id;
    
    std::string m_name;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_rotation;

    std::vector<OBJECT_ID> m_children;

    Object();
    Object(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);
    virtual ~Object();

    glm::mat4 GetLocalToWorld() const;
    virtual void Update();
    virtual void LoadFromJson(const nlohmann::json& objJson);

    void AddChild(OBJECT_ID child);

    static Object* GetObjectPtr(OBJECT_ID id);
private:

    static std::vector<Object*> s_objs;
};
