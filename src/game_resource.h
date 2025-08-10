#pragma once
#include "material_new.h"
#include "utils.h"

namespace op
{
    class GameResource : public Singleton<GameResource>
    {
    public:
        Time time;
        Event<> onFrameEnd;

        GameResource();
        ~GameResource();

        MaterialNew* GetPredefinedMaterial(size_t nameId);

    private:
        std::unordered_map<size_t, MaterialNew*> m_predefinedMaterials;
    };
}
