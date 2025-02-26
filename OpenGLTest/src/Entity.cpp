#include "Entity.h"

#include "Material.h"

Entity::~Entity()
{
    if (mesh)
    {
        DECREF(mesh);
    }

    if (material)
    {
        DECREF(material);
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
            bounds = mesh->bounds;
            INCREF(mesh);
        }
    }

    if(objJson.contains("material"))
    {
        material = Material::LoadFromFile(objJson["material"].get<std::string>());
        if (material)
        {
            INCREF(material);
        }
    }
}
