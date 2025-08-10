#include "game_resource.h"
#include "const.h"

namespace op
{
    GameResource::GameResource()
    {
        for (const auto& nameId : PREDEFINED_MATERIALS)
        {
            auto mat = new MaterialNew();
            INCREF(mat);
            
            m_predefinedMaterials[nameId] = mat;
        }
    }

    GameResource::~GameResource()
    {
        for (auto& [nameId, mat] : m_predefinedMaterials)
        {
            DECREF(mat);
        }
    }

    MaterialNew* GameResource::GetPredefinedMaterial(const size_t nameId)
    {
        auto it = m_predefinedMaterials.find(nameId);
        if (it != m_predefinedMaterials.end())
        {
            return it->second;
        }

        return nullptr;
    }
}
