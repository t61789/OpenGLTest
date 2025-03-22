#include "render_comp.h"

#include "mesh.h"
#include "material.h"
#include "object.h"

namespace op
{
    RenderComp::~RenderComp()
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

    void RenderComp::LoadFromJson(const nlohmann::json& objJson)
    {
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
}
