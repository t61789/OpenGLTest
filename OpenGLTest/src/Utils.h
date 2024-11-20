#pragma once
#include <vec3.hpp>
#include "../lib/json.hpp"

class Utils
{
public:
    static glm::vec3 ToVec3(nlohmann::json arr);
    static std::string GetRealAssetPath(const std::string& relativePath);
};
