#include "LightComp.h"

#include "Utils.h"

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
        intensity = objJson["radius"].get<float>();
    }
}
