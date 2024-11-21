#include "Utils.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vec4.hpp>

Event<GLFWwindow*, int, int> Utils::s_setFrameBufferSizeEvent;

Bounds::Bounds() = default;

Bounds::Bounds(const glm::vec3 center, const glm::vec3 extents)
{
    this->center = center;
    this->extents = extents;
}

glm::vec3 Utils::ToVec3(nlohmann::json arr)
{
    if(!IsVec3(arr))
    {
        return {};
    }
    
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
}

glm::vec4 Utils::ToVec4(nlohmann::json arr)
{
    if(!IsVec4(arr))
    {
        return {};
    }
    
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>(),
        arr[3].get<float>()
    };
}

std::string Utils::GetRealAssetPath(const std::string& relativePath)
{
    return std::string("../assets/") + relativePath;
}

void Utils::LogInfo(const std::string& msg)
{
    std::cout << "[INFO] " << msg << "\n";
}

void Utils::LogInfo(const bool& msg)
{
    LogInfo(msg ? std::string("True") : std::string("False"));
}

void Utils::LogInfo(const float& msg)
{
    LogInfo(std::to_string(msg));
}

void Utils::LogInfo(const int& msg)
{
    LogInfo(std::to_string(msg));
}

void Utils::LogInfo(const unsigned int& msg)
{
    LogInfo(std::to_string(msg));
}

void Utils::LogInfo(const size_t& msg)
{
    LogInfo(std::to_string(msg));
}

void Utils::LogInfo(const glm::vec3& msg)
{
    LogInfo(ToString(msg));
}

void Utils::LogWarning(const std::string& msg)
{
    std::cout << "[WARNING] " << msg << "\n";
}

void Utils::LogError(const std::string& msg)
{
    std::cout << "[ERROR] " << msg << "\n";
}

std::string Utils::ToString(const float val, const int fixed)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(fixed) << val;
    return oss.str();
}

std::string Utils::ToString(const glm::vec3& val)
{
    return "(" + ToString(val.x, 2) + ", " + ToString(val.y, 2) + ", " + ToString(val.z, 2) + ")";   
}

std::string Utils::ToString(const glm::vec4& val)
{
    return "(" + ToString(val.x, 2) + ", " + ToString(val.y, 2) + ", " + ToString(val.z, 2) + ", " + ToString(val.w, 2) + ")";   
}

bool Utils::IsVec(const nlohmann::json& jsonValue, const size_t components)
{
    if (jsonValue.is_array() && jsonValue.size() == components)
    {
        bool allFloat = true;
        for (auto& e : jsonValue)
        {
            allFloat &= e.is_number();
        }
        return allFloat;
    }

    return false;
}

bool Utils::IsVec3(const nlohmann::json& jsonValue)
{
    return IsVec(jsonValue, 3);
}

bool Utils::IsVec4(const nlohmann::json& jsonValue)
{
    return IsVec(jsonValue, 4);
}

bool Utils::EndsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && str.rfind(suffix) == str.size() - suffix.size();
}

