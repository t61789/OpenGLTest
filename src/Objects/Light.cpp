#include "Light.h"

#include "Utils.h"

void Light::LoadFromJson(const nlohmann::json& objJson)
{
    Object::LoadFromJson(objJson);

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
}
