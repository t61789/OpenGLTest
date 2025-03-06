#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Object.h"

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
