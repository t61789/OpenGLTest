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

std::vector<std::string> Utils::ToDirectories(const std::string& path)
{
    std::string curPath = path;
    if(!curPath.empty() && curPath[0] == '/')
    {
        curPath = curPath.substr(1);
    }
    if(!curPath.empty() && curPath[curPath.size() - 1] == '/')
    {
        curPath = curPath.substr(0, curPath.size() - 1);
    }

    std::vector<std::string> directories;
    std::string directory;
    std::stringstream ss(curPath);
    while(std::getline(ss, directory, '/'))
    {
        directories.push_back(directory);
    }

    return directories;
}

std::string Utils::GetRealAssetPath(const std::string& relativePath)
{
    return std::string("../assets/") + relativePath;
}

std::string Utils::GetRealAssetPath(const std::string& relativePath, const std::string& curPath)
{
    auto relativePathDirectories = ToDirectories(relativePath);
    auto curPathDirectories = ToDirectories(curPath);

    if(relativePathDirectories.empty() || curPathDirectories.empty())
    {
        return GetRealAssetPath(relativePath);
    }

    if(relativePathDirectories[0] != "." && relativePathDirectories[0] != "..")
    {
        return GetRealAssetPath(relativePath);
    }

    // 如果当前路径是文件，则把最后的文件名去掉
    if(curPathDirectories[curPathDirectories.size() - 1] != "." &&
        curPathDirectories[curPathDirectories.size() - 1].find(".") != std::string::npos)
    {
        curPathDirectories.pop_back();
    }

    for (const auto& dir : relativePathDirectories)
    {
        if(dir == "..")
        {
            if(curPathDirectories.empty())
            {
                return "pathError";
            }
            curPathDirectories.pop_back();
        }
        else if(dir == ".")
        {
            // do nothing
        }
        else
        {
            curPathDirectories.push_back(dir);
        }
    }

    return JoinStrings(curPathDirectories, "/");
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

std::string Utils::JoinStrings(const std::vector<std::string>& strings, std::string delimiter)
{
    std::stringstream ss;
    for (int i = 0; i < strings.size(); ++i)
    {
        ss << strings[i];
        if (i != strings.size() - 1)
        {
            ss << delimiter;
        }
    }
    return ss.str();
}
