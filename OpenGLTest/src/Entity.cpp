#include "Entity.h"

#include "Material.h"

Entity::~Entity()
{
    if (mesh)
    {
        mesh->DecRef();
    }

    if (material)
    {
        material->DecRef();
    }
}

void Entity::LoadFromJson(const nlohmann::json& objJson)
{
    Object::LoadFromJson(objJson);

    if(objJson.contains("mesh"))
    {
        mesh = Mesh::LoadFromFile(objJson["mesh"].get<std::string>());
        if (mesh)
        {
            mesh->IncRef();
        }
    }

    if(objJson.contains("material"))
    {
        material = Material::LoadFromFile(objJson["material"].get<std::string>());
        if (material)
        {
            material->IncRef();
        }
    }
}
