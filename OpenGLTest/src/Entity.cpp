#include "Entity.h"

#include "Material.h"

Entity::~Entity() = default;

void Entity::LoadFromJson(const nlohmann::json& objJson)
{
    Object::LoadFromJson(objJson);

    if(objJson.contains("mesh"))
    {
        m_mesh = Mesh::LoadFromFile(objJson["mesh"].get<std::string>());
    }

    if(objJson.contains("material"))
    {
        m_material = Material::LoadFromFile(objJson["material"].get<std::string>());
    }
}
