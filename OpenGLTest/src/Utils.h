#pragma once
#include <vec3.hpp>
#include "../lib/json.hpp"

class Utils
{
public:
    static glm::vec3 ToVec3(nlohmann::json arr);
    static std::string GetRealAssetPath(const std::string& relativePath);
    template<typename Base, typename T>
    static bool InstanceOf(const T* ptr)
    {
        return dynamic_cast<const Base*>(ptr) != nullptr;
    }
};
