#include "Utils.h"

glm::vec3 Utils::ToVec3(nlohmann::json arr)
{
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
}

