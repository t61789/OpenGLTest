#pragma once
#include "Object.h"

class Material;

class Entity : public Object
{
public:
    ~Entity() override;
    
    RESOURCE_ID m_mesh = UNDEFINED_RESOURCE;
    RESOURCE_ID m_material = UNDEFINED_RESOURCE;

private:

    void LoadFromJson(const nlohmann::json& objJson) override;
};
