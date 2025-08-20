#include "game_resource.h"
#include "const.h"
#include "shared_object.h"

namespace op
{
    GameResource::GameResource()
    {
    }

    GameResource::~GameResource()
    {
        
    }

    void GameResource::SubmitPredefinedMaterial(const size_t nameId, Material* material)
    {
        m_predefinedMaterials[nameId] = material;
    }

    Material* GameResource::GetPredefinedMaterial(const size_t nameId)
    {
        auto it = m_predefinedMaterials.find(nameId);
        if (it != m_predefinedMaterials.end())
        {
            return it->second;
        }

        return nullptr;
    }
}
