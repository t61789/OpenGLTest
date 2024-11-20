#include "Entity.h"

Entity::~Entity()
{
    delete m_mesh;
}

void Entity::LoadFromJson(const nlohmann::json& objJson)
{
    Object::LoadFromJson(objJson);

    if(objJson.contains("mesh"))
    {
        m_mesh = Mesh::LoadFromFile(objJson["mesh"].get<std::string>());
    }

    if(objJson.contains("material"))
    {
    }
}
