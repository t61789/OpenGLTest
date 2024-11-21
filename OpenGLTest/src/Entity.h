#pragma once
#include "Object.h"

class Material;

class Entity : public Object
{
public:
    ~Entity() override;
    
    RESOURCE_ID mesh = UNDEFINED_RESOURCE;
    RESOURCE_ID material = UNDEFINED_RESOURCE;

private:

    void loadFromJson(const nlohmann::json& objJson) override;
};
