#include "light_comp.h"

#include "utils.h"

namespace op
{
    Vec3 LightComp::GetColor() const
    {
        return color * intensity;
    }

    void LightComp::LoadFromJson(const nlohmann::json& objJson)
    {
        if (objJson.contains("color"))
        {
            color = Utils::ToVec3(objJson["color"]);
        }

        if (objJson.contains("light_type"))
        {
            lightType = objJson["light_type"].get<int>();
        }
    
        if (objJson.contains("intensity"))
        {
            intensity = objJson["intensity"].get<float>();
        }
    
        if (objJson.contains("radius"))
        {
            radius = objJson["radius"].get<float>();
        }
    }
}
