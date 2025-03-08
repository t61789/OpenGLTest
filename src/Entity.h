#pragma once

#include "Object.h"
#include "Bounds.h"

class Mesh;
class Material;

class Entity : public Object
{
public:
    ~Entity() override;
    
    Mesh* mesh = nullptr;
    Material* material = nullptr;

    Bounds bounds;

private:
    void LoadFromJson(const nlohmann::json& objJson) override;
};
