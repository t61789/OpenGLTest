#pragma once
#include <vec3.hpp>
#include <vec4.hpp>

#include "Event.h"
#include "../lib/json.hpp"
#include "glfw3.h"

class Bounds
{
public:
    glm::vec3 center;
    glm::vec3 extents;

    Bounds();
    Bounds(glm::vec3 center, glm::vec3 extents);
};

class Utils
{
public:
    static Event<GLFWwindow*, int, int> s_setFrameBufferSizeEvent;
    
    static glm::vec3 ToVec3(nlohmann::json arr);
    static glm::vec4 ToVec4(nlohmann::json arr);
    
    static std::string GetRealAssetPath(const std::string& relativePath);
    
    template<typename Base, typename T>
    static bool InstanceOf(const T* ptr)
    {
        return dynamic_cast<const Base*>(ptr) != nullptr;
    }
    
    static void LogInfo(const std::string& msg);
    static void LogInfo(const bool& msg);
    static void LogInfo(const float& msg);
    static void LogInfo(const int& msg);
    static void LogInfo(const unsigned int& msg);
    static void LogInfo(const size_t& msg);
    static void LogInfo(const glm::vec3& msg);
    static void LogWarning(const std::string& msg);
    static void LogError(const std::string& msg);

    static std::string ToString(float val, int fixed);
    static std::string ToString(const glm::vec3& val);
    static std::string ToString(const glm::vec4& val);

    static bool IsVec(const nlohmann::json& jsonValue, size_t components);
    static bool IsVec3(const nlohmann::json& jsonValue);
    static bool IsVec4(const nlohmann::json& jsonValue);

    static bool EndsWith(const std::string& str, const std::string& suffix);
};
