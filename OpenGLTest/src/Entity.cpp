#include "Entity.h"

#include "Material.h"

Entity::~Entity() = default;

void Entity::loadFromJson(const nlohmann::json& objJson)
{
    Object::loadFromJson(objJson);

    if(objJson.contains("mesh"))
    {
        mesh = Mesh::LoadFromFile(objJson["mesh"].get<std::string>());
    }

    if(objJson.contains("material"))
    {
        material = Material::LoadFromFile(objJson["material"].get<std::string>());
    }
}
