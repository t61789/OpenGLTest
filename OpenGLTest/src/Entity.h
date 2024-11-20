#pragma once
#include "Mesh.h"
#include "Object.h"
#include "Shader.h"

class Material;

class Entity : public Object
{
public:
    ~Entity();
    
    RESOURCE_ID m_shader = -1;
    RESOURCE_ID m_mesh = -1;
    RESOURCE_ID m_mat = -1;

private:

    void LoadFromJson(const nlohmann::json& objJson) override;
};
